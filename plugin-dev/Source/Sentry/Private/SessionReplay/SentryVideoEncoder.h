// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if defined(USE_SENTRY_SESSION_REPLAY) && defined(SENTRY_REPLAY_ENCODER_AVCODECS)

#include "Containers/Queue.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "RHIFwd.h"
#include "Templates/SharedPointer.h"

#include "SentryFMP4FragmentBuilder.h"
#include "SentryFMP4Writer.h"
#include "SentryVideoEncoderInterface.h"

#include "Video/VideoEncoder.h"

class FRunnableThread;
class FEvent;
class FRHICommandListImmediate;
class FVideoResourceRHI;

struct FSentryVideoFrame;

/**
 * Wraps the AVCodecs H.264 encoder and runs on a dedicated thread.
 *
 * Frames arrive from the render-thread capture path already written by the GPU;
 * the thread waits on each frame's fence, submits it to the hardware encoder,
 * polls for output packets and feeds the resulting access units to the shared
 * fragment builder.
 */
class FSentryVideoEncoder : public ISentryEncoder, public FRunnable
{
public:
	FSentryVideoEncoder(FSentrySessionReplayRecorder& InRecorder, const FSentryEncoderConfig& InConfig);

	virtual ~FSentryVideoEncoder() override;

	// ISentryEncoder
	virtual bool StartEncoder() override;
	virtual void StopEncoder() override;
	virtual void SubmitFrame(FRHICommandListImmediate& RHICmdList, const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& Frame) override;
	virtual ETextureCreateFlags GetFrameTextureFlags() const override;
	virtual uint32 GetFramerate() const override { return Framerate; }
	virtual uint32 GetWidth() const override { return Width; }
	virtual uint32 GetHeight() const override { return Height; }
	virtual bool IsEncodingDisabled() const override { return bEncodingDisabled; }

	// FRunnable
	virtual bool Init() override;
	virtual void Stop() override;
	virtual void Exit() override;
	virtual uint32 Run() override;

private:
	// Checks if frame dimensions match with the app's fixed screen orientation
	bool ShouldSwapDimensions(uint32 ResourceWidth, uint32 ResourceHeight) const;

	bool EnsureEncoderOpen(uint32 ResourceWidth, uint32 ResourceHeight);

	// Encodes one fence-ready frame and drains produced packets
	void ProcessFrame(FSentryVideoFrame& Frame);

	// Returns a cached FVideoResourceRHI wrapping Texture, creating one on first use
	TSharedPtr<FVideoResourceRHI> AcquireVideoResource(const FTextureRHIRef& Texture);

	// Releases every queued frame back to its pool slot and empties the queue
	void DrainAndReleaseQueue();

	// Pulls available packets from the encoder, converts them to AVCC samples and emits a fragment at each keyframe boundary
	void DrainPackets();

	// Tears down the current encoder and resets per-encoder state so the next frame
	// re-baselines against a fresh VT timestamp origin and republishes a new init
	// segment. Used to avoid uint32 overflow of the SendFrame timestamp (~71 min of
	// microseconds on Apple platforms, ~49 days of milliseconds on Windows). Must be called only
	// from the encoder thread
	void Restart();

	FSentryFMP4FragmentBuilder FragmentBuilder;

	bool bEncoderOpen = false;
	bool bResolutionChanged = false;

	// Screen orientation the app runs in, captured once at construction (iOS only)
	// and assumed constant for the session. Unknown disables orientation handling
	EDeviceScreenOrientation ExpectedOrientation = EDeviceScreenOrientation::Unknown;

	TSharedPtr<TVideoEncoder<FVideoResourceRHI>> Encoder;

	struct FCachedVideoResource
	{
		FTextureRHIRef Texture;
		TSharedPtr<FVideoResourceRHI> Resource;
	};
	TMap<FRHITexture*, FCachedVideoResource> ResourceCache;

	bool bFirstFrameValidated = false;
	FThreadSafeBool bEncodingDisabled;

	int32 ConsecutiveSendFrameFailures = 0;
	static constexpr int32 MaxConsecutiveSendFrameFailures = 30;

	// Recheck cadence (ms) while waiting for a frame's GPU write fence to signal
	static constexpr uint32 FencePollIntervalMs = 2;
	// Idle wait (ms) when there is no work: encoding disabled or an empty queue
	static constexpr uint32 IdlePollIntervalMs = 50;

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
	FCriticalSection QueueLock;
	TArray<TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>> PendingQueue;

	// Timing (encoder-thread-only)
	double CaptureTimeBaseSeconds = -1.0;
	uint32 LastPacketTimestampMs = 0;
	bool bHavePrevPacketTimestamp = false;
	double LastForcedKeyframeTime = 0.0;
};

typedef FSentryVideoEncoder FSentryEncoder;

#endif // USE_SENTRY_SESSION_REPLAY && SENTRY_REPLAY_ENCODER_AVCODECS
