// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryVideoEncoder.h"

#if USE_SENTRY_CRASH_VIDEO

#include "SentryCrashVideoSubsystem.h"
#include "SentryDefines.h"

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

#if PLATFORM_MAC
#include "Video/Encoders/Configs/VideoEncoderConfigVT.h"
#endif

FSentryVideoEncoder::FSentryVideoEncoder(
	FSentryCrashVideoSubsystem& InOwner,
	uint32 InWidth,
	uint32 InHeight,
	uint32 InFramerate,
	int32 InBitrateKbps,
	float InFragmentSeconds)
	: Owner(InOwner)
	, Width(InWidth)
	, Height(InHeight)
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

	Thread = FRunnableThread::Create(this, TEXT("SentryCrashVideoEncoder"), 0, TPri_BelowNormal);
	if (!Thread)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: failed to start encoder thread"));
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
	if (!Texture.IsValid() || bStopRequested)
	{
		return;
	}

	const uint64 TimestampUs = NextTimestampUs;
	NextTimestampUs += 1000000u / FMath::Max(1u, Framerate);

	{
		FScopeLock Lock(&QueueLock);
		// Cap the queue depth to avoid runaway memory if encoder is starved.
		const int32 MaxQueueDepth = 8;
		if (PendingQueue.Num() >= MaxQueueDepth)
		{
			PendingQueue.RemoveAt(0, 1, EAllowShrinking::No);
		}
		PendingQueue.Add({ Texture, TimestampUs });
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
	if (Encoder.IsValid())
	{
		// NOTE: AVCodecs' TVideoEncoder API documents that SendFrame(nullptr)
		// performs a flush (Video/VideoEncoder.h, comment on SendFrame).
		// However FVideoEncoderNVENCD3D12::SendFrame dereferences Resource
		// unconditionally on its very first line — Resource->GetFormat() —
		// so calling the documented flush form null-derefs and asserts
		// inside TSharedPtr::operator->. We skip the explicit flush and just
		// drain whatever's already in the output queue. With
		// LatencyMode = UltraLowLatency the encoder doesn't hold frames
		// across calls, so a missed flush only loses one in-flight frame at
		// most — acceptable for a rolling crash-video window.
		DrainPackets();
		Encoder.Reset();
	}
	bEncoderOpen = false;
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

	// Configure for H.264, CBR, ultra-low latency. KeyframeInterval is left at
	// 0 ("auto" / no upper bound) — the encoder thread forces an IDR via
	// bForceKeyframe on a wall-clock cadence so fragments are ~FragmentSeconds
	// of real time, not frames. This makes the WindowSeconds setting honour
	// the actual elapsed time regardless of the source's render rate.
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
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: failed to create H.264 encoder (no NVENC/AMF backend available?)"));
		return false;
	}

#if PLATFORM_MAC
	// Workaround for an AVCodecs bug: FVideoEncoderConfigVT::PixelFormat is
	// declared without a default initializer (VideoEncoderConfigVT.h:29) and
	// the H264-to-VT TransformConfig (VideoEncoderConfigVT.cpp:63) doesn't
	// set it. Per-frame ApplyConfig then calls GetCVPixelFormatType() on
	// uninitialised memory and the `default:` case hits `unimplemented()`.
	//
	// FAVInstance::Edit<T>() lazily creates the typed config when it doesn't
	// exist (AVInstance.h:113-122) — do NOT guard with Has<T>(), because at
	// this point right after Create the VT config hasn't been added yet
	// (AVCodecs lazily transforms it during the first SendFrame). Calling
	// Edit unconditionally seeds the config with PixelFormat = BGRA;
	// AVCodecs' subsequent TransformConfig<VT, H264> sets the other fields
	// in-place without touching PixelFormat, so our value persists.
	if (FAVInstance* Instance = Encoder->GetInstance().Get())
	{
		Instance->Edit<FVideoEncoderConfigVT>().PixelFormat = EVideoFormat::BGRA;
	}
#endif

	Width = ResourceWidth;
	Height = ResourceHeight;
	bEncoderOpen = true;
	UE_LOG(LogSentrySdk, Log, TEXT("Crash video: encoder opened %ux%u @ %u fps, %d kbps, forced keyframe every %.2fs"),
		Width, Height, Framerate, BitrateBps / 1000, FragmentSeconds);
	return true;
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
			// Encoder previously refused the input; drop any incoming frames
			// on the floor and idle until shutdown.
			if (WakeEvent)
			{
				WakeEvent->Wait(50);
			}
			continue;
		}

		for (const FPendingFrame& Frame : Frames)
		{
			if (!Frame.Texture.IsValid())
			{
				continue;
			}
			const uint32 ResW = Frame.Texture->GetSizeX();
			const uint32 ResH = Frame.Texture->GetSizeY();
			if (!EnsureEncoderOpen(ResW, ResH))
			{
				continue;
			}

			TSharedRef<FVideoResourceRHI> Resource = MakeShared<FVideoResourceRHI>(
				Encoder->GetDevice().ToSharedRef(),
				FVideoResourceRHI::FRawData{ Frame.Texture, nullptr, 0 });

			// Force a keyframe every FragmentSeconds of wall clock. This makes
			// fragment durations honour real time even when the source renders
			// slower than `Framerate` — a frame-count-based keyframe interval
			// would stretch each fragment by the same factor and inflate the
			// effective window (e.g. 12s setting → 24s file at half rate).
			const double NowSec = FPlatformTime::Seconds();
			bool bForceKeyframe = false;
			if (LastForcedKeyframeTime <= 0.0 || (NowSec - LastForcedKeyframeTime) >= FragmentSeconds)
			{
				bForceKeyframe = true;
				LastForcedKeyframeTime = NowSec;
			}

			const FAVResult Result = Encoder->SendFrame(Resource, static_cast<uint32>(Frame.TimestampUs / 1000), bForceKeyframe);
			if (Result.IsSuccess())
			{
				bFirstFrameValidated = true;
			}
			else if (!bFirstFrameValidated)
			{
				// AVCodecs rejected the first frame — likely an unsupported
				// backbuffer format for the active backend (NVENC D3D12 wants
				// BGRA8; VTCodecs takes BGRA/ABGR10). Stop trying so we don't
				// burn cycles on subsequent frames.
				UE_LOG(LogSentrySdk, Warning,
					TEXT("Crash video: encoder rejected the first frame (backbuffer format may not be supported by the active AVCodecs backend). Recording disabled for this session."));
				bEncodingDisabled.AtomicSet(true);
				break;
			}
			else
			{
				UE_LOG(LogSentrySdk, Verbose, TEXT("Crash video: SendFrame returned non-success"));
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

void FSentryVideoEncoder::DrainPackets()
{
	if (!Encoder.IsValid())
	{
		return;
	}

	FVideoPacket Packet;
	while (Encoder->ReceivePacket(Packet).IsSuccess())
	{
		++PacketsReceived;

		if (Packet.DataSize == 0 || !Packet.DataPtr.IsValid())
		{
			continue;
		}

		// Convert Annex-B → AVCC, extracting any SPS/PPS.
		TArray<uint8> Sps, Pps;
		TArray<uint8> Avcc = FSentryFmp4Writer::AnnexBToAvcc(
			Packet.DataPtr.Get(), Packet.DataSize, &Sps, &Pps);

		if (Sps.Num() > 0 && CachedSps.Num() == 0)
		{
			CachedSps = MoveTemp(Sps);
		}
		if (Pps.Num() > 0 && CachedPps.Num() == 0)
		{
			CachedPps = MoveTemp(Pps);
		}

		// Once we have SPS+PPS, publish the init segment.
		if (!bInitSegmentPublished && CachedSps.Num() > 0 && CachedPps.Num() > 0)
		{
			TArray<uint8> Init = FSentryFmp4Writer::BuildInitSegment(Width, Height, CachedSps, CachedPps);
			Owner.OnInitSegmentReady(MoveTemp(Init));
			bInitSegmentPublished = true;
		}

		if (Avcc.Num() == 0)
		{
			// Packet contained only parameter sets.
			continue;
		}

		// On every keyframe boundary, finalize the previous fragment.
		if (Packet.bIsKeyframe && CurrentSamples.Num() > 0)
		{
			FinalizeFragment();
		}

		// Wall-clock sample duration: the gap between the previous packet's
		// emission and now. The first packet of the session uses a 1/Framerate
		// fallback. This way the recorded video plays back at real time even
		// when the source renders below the configured target rate.
		const uint64 NowUs = static_cast<uint64>(FPlatformTime::Seconds() * 1000000.0);
		uint64 DurationUs;
		if (LastPacketWallClockUs == 0)
		{
			DurationUs = 1000000u / FMath::Max(1u, Framerate);
		}
		else
		{
			DurationUs = NowUs - LastPacketWallClockUs;
			// Guard against runaway durations if the encoder stalled
			// (e.g. window minimised). Clamp to <=2s.
			DurationUs = FMath::Min<uint64>(DurationUs, 2000000u);
		}
		LastPacketWallClockUs = NowUs;
		const uint32 DurationTicks = static_cast<uint32>(
			(static_cast<uint64>(FSentryFmp4Writer::TrackTimescale) * DurationUs) / 1000000u);

		FSentryH264Sample Sample;
		Sample.AvccBytes = MoveTemp(Avcc);
		Sample.Duration = FMath::Max(1u, DurationTicks);
		Sample.bIsKeyframe = Packet.bIsKeyframe != 0;
		if (CurrentSamples.Num() == 0)
		{
			CurrentFragmentDecodeTime = SampleClock;
		}
		SampleClock += Sample.Duration;
		CurrentSamples.Add(MoveTemp(Sample));

		// Hard cap: never let a fragment grow beyond ~2× target duration even if
		// keyframes are missed (e.g. encoder hiccup). A new "synthetic" fragment
		// boundary will not be a real keyframe — so we don't actually flush here;
		// instead we just rely on the encoder respecting KeyframeInterval.
	}
}

void FSentryVideoEncoder::FinalizeFragment()
{
	if (CurrentSamples.Num() == 0)
	{
		return;
	}

	TArray<uint8> Frag = FSentryFmp4Writer::BuildFragment(
		NextFragmentSequence++,
		CurrentFragmentDecodeTime,
		CurrentSamples);
	Owner.OnFragmentReady(MoveTemp(Frag));

	CurrentSamples.Reset();
}

#endif // USE_SENTRY_CRASH_VIDEO
