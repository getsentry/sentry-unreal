// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoEncoder.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryDefines.h"
#include "SentrySessionReplayRecorder.h"

#include "HAL/Event.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
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
}

void FSentryVideoEncoder::SubmitFrame(const FTextureRHIRef& Texture, double CaptureTimeSeconds)
{
	if (!Texture.IsValid() || bStopRequested || bEncodingDisabled)
	{
		return;
	}

	{
		FScopeLock Lock(&QueueLock);
		if (PendingQueue.Num() >= MaxQueueDepth)
		{
			PendingQueue.RemoveAt(0, 1, EAllowShrinking::No);
		}
		PendingQueue.Add(FPendingFrame{ Texture, CaptureTimeSeconds });
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
	Encoder.Reset();
	bEncoderOpen = false;
}

uint32 FSentryVideoEncoder::Run()
{
	while (!bStopRequested)
	{
		TArray<FPendingFrame> Frames;
		{
			FScopeLock Lock(&QueueLock);
			Swap(Frames, PendingQueue);
		}

		if (bEncodingDisabled)
		{
			if (WakeEvent)
			{
				WakeEvent->Wait(50);
			}
			continue;
		}

		for (const FPendingFrame& Frame : Frames)
		{
			const FTextureRHIRef& FrameTexture = Frame.Texture;
			if (!FrameTexture.IsValid())
			{
				continue;
			}
			const uint32 ResW = FrameTexture->GetSizeX();
			const uint32 ResH = FrameTexture->GetSizeY();
			if (!EnsureEncoderOpen(ResW, ResH))
			{
				if (bEncodingDisabled)
				{
					break;
				}
				continue;
			}

			TSharedRef<FVideoResourceRHI> Resource = MakeShared<FVideoResourceRHI>(Encoder->GetDevice().ToSharedRef(),
				FVideoResourceRHI::FRawData{ FrameTexture, nullptr, 0 });

			bool bForceKeyframe = false;
			if (LastForcedKeyframeTime <= 0.0 || (Frame.CaptureTimeSeconds - LastForcedKeyframeTime) >= FragmentSeconds)
			{
				bForceKeyframe = true;
				LastForcedKeyframeTime = Frame.CaptureTimeSeconds;
			}

			if (CaptureTimeBaseSeconds < 0.0)
			{
				CaptureTimeBaseSeconds = Frame.CaptureTimeSeconds;
			}

			// VT interprets SendFrame's timestamp as microseconds (see Engine's VideoEncoderVT.hpp)
#if PLATFORM_MAC
			static constexpr double SendTimestampScale = 1'000'000.0;
#else
			static constexpr double SendTimestampScale = 1'000.0;
#endif

			const double TimestampSeconds = FMath::Max(0.0, Frame.CaptureTimeSeconds - CaptureTimeBaseSeconds);
			const double ScaledTimestamp = TimestampSeconds * SendTimestampScale;

#if PLATFORM_MAC
			// Restart the encoder every hour of recording. On Mac this stays well clear of the
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
				break;
			}
			else
			{
				if (++ConsecutiveSendFrameFailures >= MaxConsecutiveSendFrameFailures)
				{
					UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: encoder failed %d consecutive frames. Recording disabled for this session."), ConsecutiveSendFrameFailures);
					bEncodingDisabled.AtomicSet(true);
					break;
				}

				UE_LOG(LogSentrySdk, Verbose, TEXT("Session replay: SendFrame returned non-success (%d in a row)"), ConsecutiveSendFrameFailures);
			}

			DrainPackets();
		}

		Frames.Reset();

		if (WakeEvent)
		{
			WakeEvent->Wait(50);
		}
	}
	return 0;
}

bool FSentryVideoEncoder::EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight)
{
	if (bEncoderOpen)
	{
		if ((ResourceWidth != Width || ResourceHeight != Height) && !bResolutionChanged)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: capture resolution changed from %ux%u to %ux%u; recording stays locked to the original size and may be cropped or black."),
				Width, Height, ResourceWidth, ResourceHeight);
			bResolutionChanged = true;
		}
		return true;
	}

	if (ResourceWidth == 0 || ResourceHeight == 0)
	{
		return false;
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

#if PLATFORM_MAC
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

void FSentryVideoEncoder::Restart()
{
	DrainPackets();

	if (CurrentSamples.Num() > 0 && bInitSegmentPublished)
	{
		TArray<uint8> Frag = FSentryFMP4Writer::BuildFragment(NextFragmentSequence++, CurrentFragmentDecodeTime, CurrentSamples);
		Recorder.OnFragmentReady(MoveTemp(Frag));
	}
	CurrentSamples.Reset();

	Encoder.Reset();

	bEncoderOpen = false;

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

	{
		FScopeLock Lock(&QueueLock);
		PendingQueue.Reset();
	}
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
			Recorder.OnInitSegmentReady(MoveTemp(Init));
			bInitSegmentPublished = true;
		}

		if (Avcc.Num() == 0)
		{
			continue;
		}

		if (Packet.bIsKeyframe && CurrentSamples.Num() > 0)
		{
			TArray<uint8> Frag = FSentryFMP4Writer::BuildFragment(NextFragmentSequence++, CurrentFragmentDecodeTime, CurrentSamples);
			Recorder.OnFragmentReady(MoveTemp(Frag));
			CurrentSamples.Reset();
		}

		// The encoder echoes back the capture timestamp we passed to SendFrame
		// (relative to the first frame). Sample duration is the gap to the
		// previously emitted sample, so playback follows the real capture cadence
		// rather than the bursty encoder output cadence (and stays real-time even
		// when the source renders below the configured target rate). A skipped
		// packet never updates the marker, so its interval folds into the next
		// sample. The first sample falls back to a nominal 1/Framerate
#if PLATFORM_MAC
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
