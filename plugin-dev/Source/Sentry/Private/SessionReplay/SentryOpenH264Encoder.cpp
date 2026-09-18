// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryOpenH264Encoder.h"

#if defined(USE_SENTRY_SESSION_REPLAY) && defined(SENTRY_REPLAY_ENCODER_OPENH264)

#include "SentryColorConversion.h"
#include "SentryDefines.h"
#include "SentryFMP4Writer.h"
#include "SentrySessionReplayRecorder.h"
#include "SentryVideoFrame.h"

#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "RHIGPUReadback.h"

#include "codec_api.h"
#include "codec_app_def.h"
#include "codec_def.h"

FSentryOpenH264Encoder::FSentryOpenH264Encoder(FSentrySessionReplayRecorder& InRecorder, uint32 InFramerate, int32 InBitrateKbps, float InFragmentSeconds)
	: Recorder(InRecorder)
	, Assembler(InRecorder)
	, Framerate(FMath::Max(1u, InFramerate))
	, BitrateBps(InBitrateKbps * 1000)
	, FragmentSeconds(InFragmentSeconds)
{
}

FSentryOpenH264Encoder::~FSentryOpenH264Encoder()
{
	StopEncoder();
}

bool FSentryOpenH264Encoder::StartEncoder()
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

void FSentryOpenH264Encoder::StopEncoder()
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

void FSentryOpenH264Encoder::SubmitFrame(const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& Frame)
{
	if (!Frame.IsValid() || bStopRequested || bEncodingDisabled)
	{
		if (Frame.IsValid())
		{
			Frame->Release();
		}
		return;
	}

	{
		FScopeLock Lock(&QueueLock);
		PendingQueue.Add(Frame);
	}
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

bool FSentryOpenH264Encoder::Init()
{
	return true;
}

void FSentryOpenH264Encoder::Stop()
{
	bStopRequested.AtomicSet(true);
	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}
}

void FSentryOpenH264Encoder::Exit()
{
	CloseEncoder();
}

uint32 FSentryOpenH264Encoder::Run()
{
	while (!bStopRequested)
	{
		if (bEncodingDisabled)
		{
			DrainAndReleaseQueue();
			if (WakeEvent)
			{
				WakeEvent->Wait(IdlePollIntervalMs);
			}
			continue;
		}

		TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe> Frame;
		{
			FScopeLock Lock(&QueueLock);
			if (PendingQueue.Num() > 0)
			{
				Frame = PendingQueue[0];
			}
		}

		if (!Frame.IsValid())
		{
			if (WakeEvent)
			{
				WakeEvent->Wait(IdlePollIntervalMs);
			}
			continue;
		}

		if (!Frame->IsGpuWriteComplete())
		{
			// The GPU readback hasn't landed yet. Yield rather than spin, for
			// the same reason the hardware path does
			if (WakeEvent)
			{
				WakeEvent->Wait(ReadbackPollIntervalMs);
			}
			continue;
		}

		{
			FScopeLock Lock(&QueueLock);
			if (PendingQueue.Num() > 0)
			{
				PendingQueue.RemoveAt(0, 1, EAllowShrinking::No);
			}
		}

		ProcessFrame(*Frame);

		Frame->Release();
	}

	DrainAndReleaseQueue();

	return 0;
}

void FSentryOpenH264Encoder::ProcessFrame(FSentryVideoFrame& Frame)
{
	if (!Frame.Readback.IsValid())
	{
		return;
	}

	const uint32 FrameWidth = Frame.Width;
	const uint32 FrameHeight = Frame.Height;
	if (!EnsureEncoderOpen(FrameWidth, FrameHeight))
	{
		return;
	}

	// Locked to the resolution the encoder opened with; a later size change is
	// reported once and then ignored, matching the hardware path
	if (FrameWidth != Width || FrameHeight != Height)
	{
		return;
	}

	int32 RowPitchInPixels = 0;
	const uint8* Bgra = static_cast<const uint8*>(Frame.Readback->Lock(RowPitchInPixels));
	if (Bgra == nullptr)
	{
		Frame.Readback->Unlock();
		return;
	}

	const int32 YStride = static_cast<int32>(Width);
	const int32 CStride = static_cast<int32>((Width + 1) / 2);
	const int32 ChromaHeight = static_cast<int32>((Height + 1) / 2);
	const int32 YSize = YStride * static_cast<int32>(Height);
	const int32 CSize = CStride * ChromaHeight;

	if (YuvBuffer.Num() != YSize + 2 * CSize)
	{
		YuvBuffer.SetNumUninitialized(YSize + 2 * CSize);
	}

	uint8* PlaneY = YuvBuffer.GetData();
	uint8* PlaneU = PlaneY + YSize;
	uint8* PlaneV = PlaneU + CSize;

	FSentryColorConversion::BgraToI420(Bgra, RowPitchInPixels * 4, Width, Height,
		PlaneY, PlaneU, PlaneV, YStride, CStride);

	Frame.Readback->Unlock();

	if (CaptureTimeBaseSeconds < 0.0)
	{
		CaptureTimeBaseSeconds = Frame.CaptureTimeSeconds;
	}
	const double TimestampSeconds = FMath::Max(0.0, Frame.CaptureTimeSeconds - CaptureTimeBaseSeconds);

	SSourcePicture Picture;
	FMemory::Memzero(Picture);
	Picture.iPicWidth = static_cast<int>(Width);
	Picture.iPicHeight = static_cast<int>(Height);
	Picture.iColorFormat = videoFormatI420;
	Picture.iStride[0] = YStride;
	Picture.iStride[1] = CStride;
	Picture.iStride[2] = CStride;
	Picture.pData[0] = PlaneY;
	Picture.pData[1] = PlaneU;
	Picture.pData[2] = PlaneV;
	Picture.uiTimeStamp = static_cast<long long>(TimestampSeconds * 1000.0);

	// One IDR per frame: fragments must start on a keyframe and replay runs at a
	// handful of frames per second, so per-frame keyframes keep the fragment
	// ring trivial at a modest size cost
	Encoder->ForceIntraFrame(true);

	SFrameBSInfo Info;
	FMemory::Memzero(Info);

	if (Encoder->EncodeFrame(&Picture, &Info) != cmResultSuccess)
	{
		if (++ConsecutiveEncodeFailures >= MaxConsecutiveEncodeFailures)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: encoder failed %d consecutive frames. Recording disabled for this session."), ConsecutiveEncodeFailures);
			bEncodingDisabled.AtomicSet(true);
		}
		return;
	}
	ConsecutiveEncodeFailures = 0;

	if (Info.eFrameType == videoFrameTypeSkip)
	{
		return;
	}

	// Flatten the layers into a single Annex-B access unit. openh264 lays each
	// layer's NALUs out contiguously in pBsBuf, so one append per layer is enough
	TArray<uint8> AccessUnit;
	for (int Layer = 0; Layer < Info.iLayerNum; ++Layer)
	{
		const SLayerBSInfo& LayerInfo = Info.sLayerInfo[Layer];
		int32 LayerSize = 0;
		for (int Nal = 0; Nal < LayerInfo.iNalCount; ++Nal)
		{
			LayerSize += LayerInfo.pNalLengthInByte[Nal];
		}
		if (LayerSize > 0 && LayerInfo.pBsBuf != nullptr)
		{
			AccessUnit.Append(LayerInfo.pBsBuf, LayerSize);
		}
	}

	if (AccessUnit.Num() == 0)
	{
		return;
	}

	// Sample duration is the gap to the previous sample, so playback follows the
	// real capture cadence rather than the nominal frame rate
	double DurationSeconds;
	if (LastSampleTimeSeconds < 0.0)
	{
		DurationSeconds = 1.0 / Framerate;
	}
	else
	{
		DurationSeconds = TimestampSeconds - LastSampleTimeSeconds;
	}
	DurationSeconds = FMath::Clamp(DurationSeconds, 1.0 / 1000.0, 2.0);
	LastSampleTimeSeconds = TimestampSeconds;

	const uint32 DurationTicks = FMath::Max<uint32>(1, static_cast<uint32>(DurationSeconds * FSentryFMP4Writer::TrackTimescale));

	Assembler.AddAccessUnit(AccessUnit.GetData(), AccessUnit.Num(),
		Info.eFrameType == videoFrameTypeIDR, DurationTicks);
}

bool FSentryOpenH264Encoder::EnsureEncoderOpen(uint32 FrameWidth, uint32 FrameHeight)
{
	if (bEncoderOpen)
	{
		if ((FrameWidth != Width || FrameHeight != Height) && !bResolutionChanged)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: capture resolution changed from %ux%u to %ux%u; recording stays locked to the original size."),
				Width, Height, FrameWidth, FrameHeight);
			bResolutionChanged = true;
		}
		return true;
	}

	if (FrameWidth == 0 || FrameHeight == 0)
	{
		return false;
	}

	if (WelsCreateSVCEncoder(&Encoder) != 0 || Encoder == nullptr)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to create the openh264 encoder. Recording disabled for this session."));
		bEncodingDisabled.AtomicSet(true);
		return false;
	}

	SEncParamExt Params;
	FMemory::Memzero(Params);
	Encoder->GetDefaultParams(&Params);

	Params.iUsageType = CAMERA_VIDEO_REAL_TIME;
	Params.iPicWidth = static_cast<int>(FrameWidth);
	Params.iPicHeight = static_cast<int>(FrameHeight);
	Params.iTargetBitrate = BitrateBps;
	Params.iRCMode = RC_BITRATE_MODE;
	Params.fMaxFrameRate = static_cast<float>(Framerate);
	Params.iTemporalLayerNum = 1;
	Params.iSpatialLayerNum = 1;
	Params.bEnableDenoise = false;
	Params.bEnableFrameSkip = false;

	// Single-threaded: session replay encodes a couple of frames per second, and
	// this keeps openh264's thread pool (and its CPU-count probing) out of the
	// picture entirely
	Params.iMultipleThreadIdc = 1;

	// The avcC box in the fMP4 init segment carries exactly one SPS/PPS pair, so
	// the parameter set ids must never change. openh264 defaults to
	// INCREASING_ID, which bumps pic_parameter_set_id on every IDR and leaves
	// every fragment after the first referencing a PPS the decoder has never
	// seen - the file probes as valid H.264 and decodes to nothing
	Params.eSpsPpsIdStrategy = CONSTANT_ID;

	SSpatialLayerConfig& Layer = Params.sSpatialLayers[0];
	Layer.iVideoWidth = static_cast<int>(FrameWidth);
	Layer.iVideoHeight = static_cast<int>(FrameHeight);
	Layer.fFrameRate = static_cast<float>(Framerate);
	Layer.iSpatialBitrate = BitrateBps;
	Layer.uiProfileIdc = PRO_BASELINE;

	// Signal what FSentryColorConversion actually produces, so players don't
	// have to guess. BT.709 limited range; 1 == bt709 for all three fields
	Layer.bVideoSignalTypePresent = true;
	Layer.uiVideoFormat = VF_UNDEF;
	Layer.bFullRange = false;
	Layer.bColorDescriptionPresent = true;
	Layer.uiColorPrimaries = CP_BT709;
	Layer.uiTransferCharacteristics = TRC_BT709;
	Layer.uiColorMatrix = CM_BT709;

	if (Encoder->InitializeExt(&Params) != 0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: openh264 rejected the encoder configuration (%ux%u @ %u fps, %d kbps). Recording disabled for this session."),
			FrameWidth, FrameHeight, Framerate, BitrateBps / 1000);
		WelsDestroySVCEncoder(Encoder);
		Encoder = nullptr;
		bEncodingDisabled.AtomicSet(true);
		return false;
	}

	int VideoFormat = videoFormatI420;
	Encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &VideoFormat);

	Width = FrameWidth;
	Height = FrameHeight;
	Assembler.SetDimensions(Width, Height);

	bEncoderOpen = true;

	UE_LOG(LogSentrySdk, Log, TEXT("Session replay: openh264 encoder opened %ux%u @ %u fps, %d kbps (software, every frame a keyframe)"),
		Width, Height, Framerate, BitrateBps / 1000);

	return true;
}

void FSentryOpenH264Encoder::CloseEncoder()
{
	if (Encoder != nullptr)
	{
		Encoder->Uninitialize();
		WelsDestroySVCEncoder(Encoder);
		Encoder = nullptr;
	}
	bEncoderOpen = false;
	YuvBuffer.Empty();
}

void FSentryOpenH264Encoder::DrainAndReleaseQueue()
{
	FScopeLock Lock(&QueueLock);
	for (const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& FramePtr : PendingQueue)
	{
		if (FramePtr.IsValid())
		{
			FramePtr->Release();
		}
	}
	PendingQueue.Reset();
}

#endif // USE_SENTRY_SESSION_REPLAY && SENTRY_REPLAY_ENCODER_OPENH264
