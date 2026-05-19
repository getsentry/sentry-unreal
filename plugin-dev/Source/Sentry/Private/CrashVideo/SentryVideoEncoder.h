// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_CRASH_VIDEO

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "RHIFwd.h"
#include "Templates/SharedPointer.h"

#include "SentryFmp4Writer.h"

#include "Video/VideoEncoder.h"

class FRunnableThread;
class FEvent;
class FVideoResourceRHI;

class FSentryCrashVideoSubsystem;

/**
 * Owns the AVCodecs H.264 encoder and runs on a dedicated thread.
 *
 * Frames arrive from the render-thread capture path via SubmitFrame().
 * The thread submits them to the hardware encoder, polls for output packets,
 * splits Annex-B byte streams into per-NALU AVCC samples, and groups them
 * into fragments delimited by IDR keyframes. Completed fragments are pushed
 * back to the owning subsystem via FSentryCrashVideoSubsystem::OnFragmentReady.
 */
class FSentryVideoEncoder : public FRunnable
{
public:
	FSentryVideoEncoder(
		FSentryCrashVideoSubsystem& InOwner,
		uint32 InWidth,
		uint32 InHeight,
		uint32 InFramerate,
		int32 InBitrateKbps,
		float InFragmentSeconds);

	virtual ~FSentryVideoEncoder() override;

	bool StartEncoder();
	void StopEncoder();

	// Render thread: enqueue an RHI texture for encoding. Takes a shared ref
	// so the texture stays alive until the encoder is done with it.
	void SubmitFrame(const FTextureRHIRef& Texture);

	uint32 GetWidth() const { return Width; }
	uint32 GetHeight() const { return Height; }
	uint32 GetFramerate() const { return Framerate; }

	// FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	struct FPendingFrame
	{
		FTextureRHIRef Texture;
		uint64 TimestampUs;
	};

	bool EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight);
	void DrainPackets();
	void FinalizeFragment();

	FSentryCrashVideoSubsystem& Owner;

	uint32 Width;
	uint32 Height;
	uint32 Framerate;
	int32 BitrateBps;
	float FragmentSeconds;
	uint32 KeyframeIntervalFrames;

	FRunnableThread* Thread = nullptr;
	FEvent* WakeEvent = nullptr;
	FThreadSafeBool bStopRequested;

	FCriticalSection QueueLock;
	TArray<FPendingFrame> PendingQueue;

	uint64 NextTimestampUs = 0;
	uint64 PacketsReceived = 0;
	uint64 LastPacketWallClockUs = 0;

	bool bEncoderOpen = false;
	TSharedPtr<TVideoEncoder<FVideoResourceRHI>> Encoder;

	// Fragment-in-progress state
	TArray<FSentryH264Sample> CurrentSamples;
	uint64 CurrentFragmentDecodeTime = 0;
	uint32 NextFragmentSequence = 1;
	uint64 SampleClock = 0;            // monotonic decode time in TrackTimescale ticks
	TArray<uint8> CachedSps;
	TArray<uint8> CachedPps;
	bool bInitSegmentPublished = false;
};

#endif // USE_SENTRY_CRASH_VIDEO
