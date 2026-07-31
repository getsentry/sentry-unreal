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

#include <atomic>

class FRunnableThread;
class FEvent;
class FVideoResourceRHI;

class FSentrySessionReplayRecorder;

/**
 * One reusable capture-pool slot shared by the render and encoder threads.
 *
 * The render thread marks a slot in flight before writing it. The encoder
 * thread releases it only after the GPU write fence has passed and SendFrame
 * has finished consuming the texture.
 */
struct FSentryVideoFrame
{
	bool TryAcquire()
	{
		return !bInFlight.exchange(true, std::memory_order_acq_rel);
	}

	void Release()
	{
		bInFlight.store(false, std::memory_order_release);
	}

	FTextureRHIRef Texture;
	FGPUFenceRHIRef ReadyFence;

private:
	std::atomic<bool> bInFlight{ false };
};

using FSentryVideoFramePtr = TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>;

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

	// Enqueues a capture-pool slot for the encoder thread to process
	void SubmitFrame(const FSentryVideoFramePtr& Frame, double CaptureTimeSeconds);

	uint32 GetFramerate() const { return Framerate; }

	bool IsEncodingDisabled() const { return bEncodingDisabled; }

	// FRunnable
	virtual bool Init() override;
	virtual void Stop() override;
	virtual void Exit() override;
	virtual uint32 Run() override;

	// Frames buffered for the encoder thread
	static constexpr int32 MaxQueueDepth = 5;

private:
	// Checks if frame dimensions match with the app's fixed screen orientation
	bool ShouldSwapDimensions(uint32 ResourceWidth, uint32 ResourceHeight) const;

	bool EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight);

	// Reuses the AVCodecs wrapper and its native resource mapping for each pool texture
	TSharedPtr<FVideoResourceRHI> GetOrCreateResource(const FTextureRHIRef& Texture);

	// Moves queued slots to the retired list without making them reusable.
	// A retired slot is released only after its GPU write fence passes.
	void RetirePendingFrames();
	int32 PollRetiredFrames();
	void WaitForRetiredFrames();

	// Pulls available packets from the encoder, converts them to AVCC samples and emits a fragment at each keyframe boundary
	void DrainPackets();

	// Builds a fragment from the accumulated samples and hands it to the recorder
	void FlushCurrentFragment();

	// Tears down the current encoder and resets per-encoder state so the next frame
	// re-baselines against a fresh VT timestamp origin and republishes a new init
	// segment. Used to avoid uint32 overflow of the SendFrame timestamp (~71 min of
	// microseconds on Apple platforms, ~49 days of milliseconds on Windows). Must be called only
	// from the encoder thread
	void Restart();

	FSentrySessionReplayRecorder& Recorder;

	bool bEncoderOpen = false;
	bool bResolutionChanged = false;

	// Screen orientation the app runs in, captured once at construction (iOS only)
	// and assumed constant for the session. Unknown disables orientation handling
	EDeviceScreenOrientation ExpectedOrientation = EDeviceScreenOrientation::Unknown;

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
		FSentryVideoFramePtr Frame;
		double CaptureTimeSeconds = 0.0;
	};

	FCriticalSection QueueLock;
	TArray<FPendingFrame> PendingQueue;
	TArray<FSentryVideoFramePtr> RetiredFrames;

	// Encoder-thread-only cache. Keeping the wrapper alive also keeps AVCodecs'
	// native D3D/Vulkan/Metal mapping alive instead of rebuilding it every frame.
	// The cache is reset whenever the native input dimensions change.
	TMap<FRHITexture*, TSharedPtr<FVideoResourceRHI>> ResourceCache;
	uint32 ResourceCacheWidth = 0;
	uint32 ResourceCacheHeight = 0;

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
