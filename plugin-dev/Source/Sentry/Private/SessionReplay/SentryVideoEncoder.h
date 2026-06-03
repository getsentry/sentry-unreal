// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "RHIFwd.h"
#include "Templates/SharedPointer.h"

#include "SentryFMP4Writer.h"

#include "Video/VideoEncoder.h"

class FRunnableThread;
class FEvent;
class FVideoResourceRHI;

class FSentrySessionReplayRecorder;

/**
 * Wraps the AVCodecs H.264 encoder and runs on a dedicated thread.
 *
 * Frames arrive from the render-thread capture path via SubmitFrame().
 * The thread submits them to the hardware encoder, polls for output packets,
 * splits Annex-B byte streams into per-NALU AVCC samples, and groups them
 * into fragments delimited by IDR keyframes. Completed fragments are pushed
 * back to the owning recorder via FSentrySessionReplayRecorder::OnFragmentReady.
 */
class FSentryVideoEncoder : public FRunnable
{
public:
	FSentryVideoEncoder(FSentrySessionReplayRecorder& InRecorder, uint32 InFramerate, int32 InBitrateKbps, float InFragmentSeconds);

	virtual ~FSentryVideoEncoder() override;

	bool StartEncoder();
	void StopEncoder();

	// Enqueues a texture for the encoder thread to process
	void SubmitFrame(const FTextureRHIRef& Texture, double CaptureTimeSeconds);

	uint32 GetFramerate() const { return Framerate; }

	// FRunnable
	virtual bool Init() override;
	virtual void Stop() override;
	virtual void Exit() override;
	virtual uint32 Run() override;

	// Frames buffered for the encoder thread
	static constexpr int32 MaxQueueDepth = 5;

private:
	bool EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight);

	// Pulls available packets from the encoder, converts them to AVCC samples and emits a fragment at each keyframe boundary
	void DrainPackets();

	FSentrySessionReplayRecorder& Recorder;

	bool bEncoderOpen = false;
	bool bResolutionChanged = false;
	TSharedPtr<TVideoEncoder<FVideoResourceRHI>> Encoder;

	bool bFirstFrameValidated = false;
	FThreadSafeBool bEncodingDisabled;

	int32 ConsecutiveSendFrameFailures = 0;
	static constexpr int32 MaxConsecutiveSendFrameFailures = 30;

	// Capture config
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Framerate;
	int32 BitrateBps;
	float FragmentSeconds;

	// Encoder worker thread
	FRunnableThread* Thread = nullptr;
	FEvent* WakeEvent = nullptr;
	FThreadSafeBool bStopRequested;

	// Encoder thread frame queue
	struct FPendingFrame
	{
		FTextureRHIRef Texture;
		double CaptureTimeSeconds = 0.0;
	};

	FCriticalSection QueueLock;
	TArray<FPendingFrame> PendingQueue;

	// Timing (encoder-thread-only)
	double CaptureTimeBaseSeconds = -1.0;
	uint32 LastPacketTimestampMs = 0;
	bool bHavePrevPacketTimestamp = false;
	double LastForcedKeyframeTime = 0.0;

	// Fragment-in-progress state
	TArray<FSentryH264Sample> CurrentSamples;
	uint64 CurrentFragmentDecodeTime = 0;
	uint32 NextFragmentSequence = 1;
	uint64 SampleClock = 0;
	TArray<uint8> CachedSps;
	TArray<uint8> CachedPps;
	bool bInitSegmentPublished = false;
};

#endif // USE_SENTRY_SESSION_REPLAY
