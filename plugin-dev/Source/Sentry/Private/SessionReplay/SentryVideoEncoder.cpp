// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoEncoder.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryDefines.h"
#include "SentrySessionReplayRecorder.h"

#include "HAL/Event.h"
#include "HAL/PlatformTime.h"
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

void FSentryVideoEncoder::SubmitFrame(const FTextureRHIRef& Texture)
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
		PendingQueue.Add(Texture);
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
		TArray<FTextureRHIRef> Frames;
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

		for (const FTextureRHIRef& FrameTexture : Frames)
		{
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

			TSharedRef<FVideoResourceRHI> Resource = MakeShared<FVideoResourceRHI>(Encoder->GetDevice().ToSharedRef(),
				FVideoResourceRHI::FRawData{ FrameTexture, nullptr, 0 });

			const double NowSec = FPlatformTime::Seconds();
			bool bForceKeyframe = false;
			if (LastForcedKeyframeTime <= 0.0 || (NowSec - LastForcedKeyframeTime) >= FragmentSeconds)
			{
				bForceKeyframe = true;
				LastForcedKeyframeTime = NowSec;
			}

			const uint32 TimestampMs = static_cast<uint32>(EncodedFrameCount * 1000u / FMath::Max(1u, Framerate));
			++EncodedFrameCount;

			const FAVResult Result = Encoder->SendFrame(Resource, TimestampMs, bForceKeyframe);
			if (Result.IsSuccess())
			{
				bFirstFrameValidated = true;
			}
			else if (!bFirstFrameValidated)
			{
				UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: encoder rejected the first frame. Recording disabled for this session."));
				bEncodingDisabled.AtomicSet(true);
				break;
			}
			else
			{
				UE_LOG(LogSentrySdk, Verbose, TEXT("Session replay: SendFrame returned non-success"));
			}

			DrainPackets();
		}

		if (PendingQueue.Num() == 0 && WakeEvent)
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

	TSharedRef<FAVDevice>& Device = FAVDevice::GetHardwareDevice();

	Encoder = FVideoEncoder::Create<FVideoResourceRHI>(Device, Config);
	if (!Encoder.IsValid())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to create H.264 encoder"));
		return false;
	}

	Width = ResourceWidth;
	Height = ResourceHeight;

	bEncoderOpen = true;

	UE_LOG(LogSentrySdk, Log, TEXT("Session replay: encoder opened %ux%u @ %u fps, %d kbps, forced keyframe every %.2fs"),
		Width, Height, Framerate, BitrateBps / 1000, FragmentSeconds);

	return true;
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

		// Wall-clock sample duration: the gap between the previous packet's
		// emission and now. The first packet of the session uses a 1/Framerate
		// fallback. This way the recorded video plays back at real time even
		// when the source renders below the configured target rate
		const double Now = FPlatformTime::Seconds();
		double DurationSeconds = (LastPacketTime <= 0.0) ? (1.0 / FMath::Max(1u, Framerate)) : (Now - LastPacketTime);

		// Guard against runaway durations if the encoder stalled (e.g. window minimised)
		DurationSeconds = FMath::Min(DurationSeconds, 2.0);

		LastPacketTime = Now;

		const uint32 DurationTicks = FMath::Max<uint32>(1, static_cast<uint32>(DurationSeconds * FSentryFMP4Writer::TrackTimescale));

		FSentryH264Sample Sample;
		Sample.AvccBytes = MoveTemp(Avcc);
		Sample.Duration = DurationTicks;
		Sample.bIsKeyframe = Packet.bIsKeyframe != 0;

		if (CurrentSamples.Num() == 0)
		{
			CurrentFragmentDecodeTime = SampleClock;
		}
		SampleClock += Sample.Duration;

		CurrentSamples.Add(MoveTemp(Sample));
	}
}

#endif // USE_SENTRY_SESSION_REPLAY
