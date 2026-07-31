// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoEncoder.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryDefines.h"
#include "SentrySessionReplayRecorder.h"

#include "HAL/Event.h"
#include "HAL/PlatformMisc.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Misc/ScopeExit.h"
#include "RHI.h"

#include "AVConfig.h"
#include "AVContext.h"
#include "AVDevice.h"
#include "AVResult.h"
#include "Video/Encoders/Configs/VideoEncoderConfigH264.h"
#include "Video/Resources/VideoResourceRHI.h"
#include "Video/VideoEncoder.h"
#include "Video/VideoPacket.h"

FSentryVideoEncoder::FSentryVideoEncoder(FSentrySessionReplayRecorder& InRecorder, uint32 InFramerate, int32 InBitrateKbps, float InFragmentSeconds)
	: Recorder(InRecorder)
	, Framerate(InFramerate)
	, BitrateBps(InBitrateKbps * 1000)
	, FragmentSeconds(InFragmentSeconds)
{
#if PLATFORM_IOS
	ExpectedOrientation = FPlatformMisc::GetDeviceOrientation();
#endif
}

FSentryVideoEncoder::~FSentryVideoEncoder()
{
	StopEncoder();
}

bool FSentryVideoEncoder::StartEncoder()
{
	if (Thread != nullptr)
	{
		return true;
	}

	WakeEvent = FPlatformProcess::GetSynchEventFromPool(false);

	bStopRequested.AtomicSet(false);

	Thread = FRunnableThread::Create(this, TEXT("SentrySessionReplayEncoder"), 0, TPri_BelowNormal);
	if (!Thread)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to start encoder thread"));

		FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
		WakeEvent = nullptr;

		return false;
	}

	return true;
}

void FSentryVideoEncoder::StopEncoder()
{
	if (Thread != nullptr)
	{
		bStopRequested.AtomicSet(true);

		if (WakeEvent)
		{
			WakeEvent->Trigger();
		}

		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
	if (WakeEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
		WakeEvent = nullptr;
	}

	RetirePendingFrames();
	PollRetiredFrames();
	ResourceCache.Reset();
}

void FSentryVideoEncoder::SubmitFrame(const FSentryVideoFramePtr& Frame, double CaptureTimeSeconds)
{
	if (!Frame.IsValid())
	{
		return;
	}

	if (!Frame->Texture.IsValid() || !Frame->ReadyFence.IsValid())
	{
		Frame->Release();
		return;
	}

	{
		FScopeLock Lock(&QueueLock);
		if (bStopRequested || bEncodingDisabled)
		{
			RetiredFrames.Add(Frame);
			return;
		}

		if (PendingQueue.Num() >= MaxQueueDepth)
		{
			if (PendingQueue[0].Frame.IsValid())
			{
				RetiredFrames.Add(MoveTemp(PendingQueue[0].Frame));
			}
			PendingQueue.RemoveAt(0, 1, EAllowShrinking::No);
		}
		PendingQueue.Add(FPendingFrame{ Frame, CaptureTimeSeconds });
	}
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

bool FSentryVideoEncoder::Init()
{
	return true;
}

void FSentryVideoEncoder::Stop()
{
	bStopRequested.AtomicSet(true);
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

void FSentryVideoEncoder::Exit()
{
	ResourceCache.Reset();
	Encoder.Reset();
	bEncoderOpen = false;
}

uint32 FSentryVideoEncoder::Run()
{
	while (!bStopRequested)
	{
		PollRetiredFrames();

		if (bEncodingDisabled)
		{
			RetirePendingFrames();
			if (WakeEvent)
			{
				WakeEvent->Wait(50);
			}
			continue;
		}

		FPendingFrame PendingFrame;
		bool bQueueNotEmpty = false;
		{
			FScopeLock Lock(&QueueLock);
			bQueueNotEmpty = !PendingQueue.IsEmpty();
			if (bQueueNotEmpty)
			{
				const FSentryVideoFramePtr& FirstFrame = PendingQueue[0].Frame;
				if (!FirstFrame.IsValid() || !FirstFrame->ReadyFence.IsValid() || FirstFrame->ReadyFence->Poll())
				{
					PendingFrame = MoveTemp(PendingQueue[0]);
					PendingQueue.RemoveAt(0, 1, EAllowShrinking::No);
				}
			}
		}

		if (!PendingFrame.Frame.IsValid())
		{
			if (WakeEvent)
			{
				// A queued frame whose fence has not passed is checked again
				// shortly. Polling from this worker never blocks the RHI thread.
				WakeEvent->Wait(bQueueNotEmpty ? 1 : 50);
			}
			continue;
		}

		ON_SCOPE_EXIT
		{
			PendingFrame.Frame->Release();
		};

		const FTextureRHIRef& FrameTexture = PendingFrame.Frame->Texture;
		if (!FrameTexture.IsValid())
		{
			continue;
		}

		const uint32 ResW = FrameTexture->GetSizeX();
		const uint32 ResH = FrameTexture->GetSizeY();
		if (!EnsureEncoderOpen(ResW, ResH))
		{
			continue;
		}

		TSharedPtr<FVideoResourceRHI> Resource = GetOrCreateResource(FrameTexture);
		if (!Resource.IsValid())
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to wrap capture texture for the video encoder"));
			continue;
		}

		bool bForceKeyframe = false;
		if (LastForcedKeyframeTime <= 0.0 || (PendingFrame.CaptureTimeSeconds - LastForcedKeyframeTime) >= FragmentSeconds)
		{
			bForceKeyframe = true;
			LastForcedKeyframeTime = PendingFrame.CaptureTimeSeconds;
		}

		if (CaptureTimeBaseSeconds < 0.0)
		{
			CaptureTimeBaseSeconds = PendingFrame.CaptureTimeSeconds;
		}

		// VT interprets SendFrame's timestamp as microseconds (see Engine's VideoEncoderVT.hpp)
#if PLATFORM_APPLE
		static constexpr double SendTimestampScale = 1'000'000.0;
#else
		static constexpr double SendTimestampScale = 1'000.0;
#endif

		const double TimestampSeconds = FMath::Max(0.0, PendingFrame.CaptureTimeSeconds - CaptureTimeBaseSeconds);
		const double ScaledTimestamp = TimestampSeconds * SendTimestampScale;

#if PLATFORM_APPLE
		// Restart the encoder every hour of recording. On Apple platforms this stays well clear of the
		// uint32-microseconds wrap at ~71 min which would otherwise feed VT a backward PTS
		// and corrupt all subsequent fragments. On Windows the natural wrap is at ~49 days
		// so realistically periodic refresh of encoder state won't be needed there
		constexpr double RestartThreshold = 3600.0 * SendTimestampScale;
		if (ScaledTimestamp > RestartThreshold)
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Session replay: encoder has been running for %.0f s; restarting to refresh state."), TimestampSeconds);
			Restart();
			continue;
		}
#endif

		const uint32 SendTimestamp = static_cast<uint32>(ScaledTimestamp);

		const FAVResult Result = Encoder->SendFrame(Resource, SendTimestamp, bForceKeyframe);
		if (Result.IsSuccess())
		{
			bFirstFrameValidated = true;
			ConsecutiveSendFrameFailures = 0;
		}
		else if (!bFirstFrameValidated)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: encoder rejected the first frame. Recording disabled for this session."));
			bEncodingDisabled.AtomicSet(true);
			continue;
		}
		else
		{
			if (++ConsecutiveSendFrameFailures >= MaxConsecutiveSendFrameFailures)
			{
				UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: encoder failed %d consecutive frames. Recording disabled for this session."), ConsecutiveSendFrameFailures);
				bEncodingDisabled.AtomicSet(true);
				continue;
			}

			UE_LOG(LogSentrySdk, Verbose, TEXT("Session replay: SendFrame returned non-success (%d in a row)"), ConsecutiveSendFrameFailures);
		}

		DrainPackets();
	}

	RetirePendingFrames();
	WaitForRetiredFrames();
	return 0;
}

bool FSentryVideoEncoder::ShouldSwapDimensions(uint32 ResourceWidth, uint32 ResourceHeight) const
{
	switch (ExpectedOrientation)
	{
	case EDeviceScreenOrientation::Portrait:
	case EDeviceScreenOrientation::PortraitUpsideDown:
		return ResourceWidth > ResourceHeight;
	case EDeviceScreenOrientation::LandscapeLeft:
	case EDeviceScreenOrientation::LandscapeRight:
		return ResourceHeight > ResourceWidth;
	default:
		return false;
	}
}

bool FSentryVideoEncoder::EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight)
{
	if (bEncoderOpen)
	{
		const bool bSameSize = ResourceWidth == Width && ResourceHeight == Height;

		// Transposed frames are expected only when orientation tracking is active (iOS);
		// elsewhere a transposed size is a genuine resolution change
		const bool bSameTransposedSize = ExpectedOrientation != EDeviceScreenOrientation::Unknown &&
										 ResourceWidth == Height && ResourceHeight == Width;

		if (!bSameSize && !bSameTransposedSize)
		{
			if (!bResolutionChanged)
			{
				UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: capture resolution changed from %ux%u to %ux%u; recording stays locked to the original size and may be cropped or black."),
					Width, Height, ResourceWidth, ResourceHeight);
				bResolutionChanged = true;
			}
		}
		return true;
	}

	if (ResourceWidth == 0 || ResourceHeight == 0)
	{
		return false;
	}

	// If the first frame arrives in the device's native (transposed) dimensions
	// swap them so the video is written with the correct resolution from the start
	if (ShouldSwapDimensions(ResourceWidth, ResourceHeight))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Session replay: first frame is %ux%u but the app runs in %s orientation; opening the encoder with swapped dimensions."),
			ResourceWidth, ResourceHeight, ResourceHeight > ResourceWidth ? TEXT("landscape") : TEXT("portrait"));
		Swap(ResourceWidth, ResourceHeight);
	}

	FVideoEncoderConfigH264 Config;
	Config.Width = ResourceWidth;
	Config.Height = ResourceHeight;
	Config.TargetFramerate = Framerate;
	Config.TargetBitrate = BitrateBps;
	Config.MaxBitrate = BitrateBps;
	Config.MinBitrate = BitrateBps / 2;
	Config.RateControlMode = ERateControlMode::CBR;
	Config.LatencyMode = EAVLatencyMode::UltraLowLatency;
	Config.KeyframeInterval = 0;
	Config.Profile = EH264Profile::Main;
	Config.RepeatSPSPPS = true;
	Config.bFillData = 0;
	Config.MultipassMode = EMultipassMode::Disabled;

#if PLATFORM_APPLE
	// Work around a VT bug where H.264 Auto maps to a null EntropyCodingMode
	// causing a crash in CFStringGetLength. Use CABAC instead (supported by Main/High profiles)
	Config.EntropyCodingMode = EH264EntropyCodingMode::CABAC;
#endif

	TSharedRef<FAVDevice>& Device = FAVDevice::GetHardwareDevice();

	Encoder = FVideoEncoder::Create<FVideoResourceRHI>(Device, Config);
	if (!Encoder.IsValid())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to create H.264 encoder - check that a codec plugin matching the GPU vendor is enabled. Recording disabled for this session."));
		bEncodingDisabled.AtomicSet(true);
		return false;
	}

	Width = ResourceWidth;
	Height = ResourceHeight;

	bEncoderOpen = true;

	UE_LOG(LogSentrySdk, Log, TEXT("Session replay: encoder opened %ux%u @ %u fps, %d kbps, forced keyframe every %.2fs"),
		Width, Height, Framerate, BitrateBps / 1000, FragmentSeconds);

	return true;
}

TSharedPtr<FVideoResourceRHI> FSentryVideoEncoder::GetOrCreateResource(const FTextureRHIRef& Texture)
{
	const uint32 TextureWidth = Texture->GetSizeX();
	const uint32 TextureHeight = Texture->GetSizeY();
	if (TextureWidth != ResourceCacheWidth || TextureHeight != ResourceCacheHeight)
	{
		ResourceCache.Reset();
		ResourceCacheWidth = TextureWidth;
		ResourceCacheHeight = TextureHeight;
	}

	TSharedPtr<FVideoResourceRHI>& Resource = ResourceCache.FindOrAdd(Texture.GetReference());
	if (!Resource.IsValid())
	{
		Resource = MakeShared<FVideoResourceRHI>(Encoder->GetDevice().ToSharedRef(),
			FVideoResourceRHI::FRawData{ Texture, nullptr, 0 });
	}
	return Resource;
}

void FSentryVideoEncoder::RetirePendingFrames()
{
	FScopeLock Lock(&QueueLock);
	for (FPendingFrame& PendingFrame : PendingQueue)
	{
		if (PendingFrame.Frame.IsValid())
		{
			RetiredFrames.Add(MoveTemp(PendingFrame.Frame));
		}
	}
	PendingQueue.Reset();
}

int32 FSentryVideoEncoder::PollRetiredFrames()
{
	FScopeLock Lock(&QueueLock);
	for (int32 Index = RetiredFrames.Num() - 1; Index >= 0; --Index)
	{
		const FSentryVideoFramePtr& Frame = RetiredFrames[Index];
		if (!Frame.IsValid() || !Frame->ReadyFence.IsValid() || Frame->ReadyFence->Poll())
		{
			if (Frame.IsValid())
			{
				Frame->Release();
			}
			RetiredFrames.RemoveAtSwap(Index, 1, EAllowShrinking::No);
		}
	}
	return RetiredFrames.Num();
}

void FSentryVideoEncoder::WaitForRetiredFrames()
{
	// Capture is stopped and its render commands are flushed before the encoder
	// is joined. Give the RHI thread time to finish any discarded frame writes
	// without ever exposing those slots for reuse while their fences are pending.
	const double Deadline = FPlatformTime::Seconds() + 5.0;
	while (PollRetiredFrames() > 0 && FPlatformTime::Seconds() < Deadline)
	{
		FPlatformProcess::SleepNoStats(0.001f);
	}

	const int32 Remaining = PollRetiredFrames();
	if (Remaining > 0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: %d retired capture frame(s) still have pending GPU fences during shutdown."), Remaining);
	}
}

void FSentryVideoEncoder::Restart()
{
	DrainPackets();

	FlushCurrentFragment();

	ResourceCache.Reset();
	ResourceCacheWidth = 0;
	ResourceCacheHeight = 0;
	Encoder.Reset();

	bEncoderOpen = false;
	bResolutionChanged = false;

	CaptureTimeBaseSeconds = -1.0;
	LastPacketTimestampMs = 0;
	bHavePrevPacketTimestamp = false;
	LastForcedKeyframeTime = 0.0;
	CurrentFragmentDecodeTime = 0;
	SampleClock = 0;
	NextFragmentSequence = 1;
	CachedSps.Empty();
	CachedPps.Empty();
	bInitSegmentPublished = false;
	bFirstFrameValidated = false;
	ConsecutiveSendFrameFailures = 0;

	RetirePendingFrames();
}

void FSentryVideoEncoder::FlushCurrentFragment()
{
	if (CurrentSamples.Num() > 0 && bInitSegmentPublished)
	{
		const uint32 FrameCount = static_cast<uint32>(CurrentSamples.Num());
		uint64 DurationTicks = 0;
		for (const FSentryH264Sample& Sample : CurrentSamples)
		{
			DurationTicks += Sample.Duration;
		}
		TArray<uint8> Fragment = FSentryFMP4Writer::BuildFragment(NextFragmentSequence++, CurrentFragmentDecodeTime, CurrentSamples);
		Recorder.OnFragmentReady(MoveTemp(Fragment), FrameCount, DurationTicks);
	}
	CurrentSamples.Reset();
}

void FSentryVideoEncoder::DrainPackets()
{
	if (!Encoder.IsValid())
	{
		return;
	}

	FVideoPacket Packet;
	while (Encoder->ReceivePacket(Packet).IsSuccess())
	{
		if (Packet.DataSize == 0 || !Packet.DataPtr.IsValid())
		{
			continue;
		}

		TArray<uint8> Sps, Pps;
		TArray<uint8> Avcc = FSentryFMP4Writer::AnnexBToAvcc(Packet.DataPtr.Get(), Packet.DataSize, &Sps, &Pps);

		if (Sps.Num() > 0 && CachedSps.Num() == 0)
		{
			CachedSps = MoveTemp(Sps);
		}
		if (Pps.Num() > 0 && CachedPps.Num() == 0)
		{
			CachedPps = MoveTemp(Pps);
		}

		if (!bInitSegmentPublished && CachedSps.Num() > 0 && CachedPps.Num() > 0)
		{
			TArray<uint8> Init = FSentryFMP4Writer::BuildInitSegment(Width, Height, CachedSps, CachedPps);
			Recorder.OnInitSegmentReady(MoveTemp(Init), Width, Height);
			bInitSegmentPublished = true;
		}

		if (Avcc.Num() == 0)
		{
			continue;
		}

		if (Packet.bIsKeyframe && CurrentSamples.Num() > 0)
		{
			FlushCurrentFragment();
		}

		// The encoder echoes back the capture timestamp we passed to SendFrame
		// (relative to the first frame). Sample duration is the gap to the
		// previously emitted sample, so playback follows the real capture cadence
		// rather than the bursty encoder output cadence (and stays real-time even
		// when the source renders below the configured target rate). A skipped
		// packet never updates the marker, so its interval folds into the next
		// sample. The first sample falls back to a nominal 1/Framerate
#if PLATFORM_APPLE
		const uint32 PacketTimestampMs = static_cast<uint32>(FPlatformTime::ToMilliseconds64(Packet.Timestamp));
#else
		const uint32 PacketTimestampMs = static_cast<uint32>(Packet.Timestamp);
#endif
		double DurationSeconds;
		if (!bHavePrevPacketTimestamp)
		{
			DurationSeconds = 1.0 / FMath::Max(1u, Framerate);
		}
		else
		{
			const uint32 DeltaMs = PacketTimestampMs - LastPacketTimestampMs;
			DurationSeconds = DeltaMs / 1000.0;
		}

		// Guard against runaway durations if the encoder stalled (e.g. window minimised)
		DurationSeconds = FMath::Min(DurationSeconds, 2.0);

		LastPacketTimestampMs = PacketTimestampMs;
		bHavePrevPacketTimestamp = true;

		const uint32 DurationTicks = FMath::Max<uint32>(1, static_cast<uint32>(DurationSeconds * FSentryFMP4Writer::TrackTimescale));

		FSentryH264Sample Sample;
		Sample.AvccBytes = MoveTemp(Avcc);
		Sample.Duration = DurationTicks;

		if (CurrentSamples.Num() == 0)
		{
			CurrentFragmentDecodeTime = SampleClock;
		}
		SampleClock += Sample.Duration;

		CurrentSamples.Add(MoveTemp(Sample));
	}
}

#endif // USE_SENTRY_SESSION_REPLAY
