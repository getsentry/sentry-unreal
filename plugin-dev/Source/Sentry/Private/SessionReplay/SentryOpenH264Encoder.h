// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if defined(USE_SENTRY_SESSION_REPLAY) && defined(SENTRY_REPLAY_ENCODER_OPENH264)

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Misc/ScopeLock.h"

#include "ISentryVideoEncoder.h"
#include "SentryReplayFragmentAssembler.h"

class FRunnableThread;
class FEvent;
class ISVCEncoder;

struct FSentryVideoFrame;
class FSentrySessionReplayRecorder;

/**
 * Software H.264 encoder built on openh264, for platforms that expose no video
 * encoder of their own (Xbox).
 *
 * Runs on a dedicated thread. Frames arrive from the render thread already
 * queued for GPU readback; the thread waits for each readback to land, converts
 * BGRA to I420 on the CPU, encodes, and hands the resulting access units to the
 * shared fragment assembler.
 *
 * Every frame is forced to an IDR, because session replay records at a very low
 * frame rate and each fMP4 fragment has to begin on a keyframe. At 1-2 fps the
 * extra cost is small and it keeps fragment boundaries trivial.
 */
class FSentryOpenH264Encoder : public ISentryVideoEncoder, public FRunnable
{
public:
	FSentryOpenH264Encoder(FSentrySessionReplayRecorder& InRecorder, uint32 InFramerate, int32 InBitrateKbps, float InFragmentSeconds);
	virtual ~FSentryOpenH264Encoder() override;

	// ISentryVideoEncoder
	virtual bool StartEncoder() override;
	virtual void StopEncoder() override;
	virtual void SubmitFrame(const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& Frame) override;
	virtual uint32 GetFramerate() const override { return Framerate; }
	virtual uint32 GetWidth() const override { return Width; }
	virtual uint32 GetHeight() const override { return Height; }
	virtual bool IsEncodingDisabled() const override { return bEncodingDisabled; }
	virtual bool RequiresCpuReadableFrames() const override { return true; }

	// FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	// Opens the openh264 encoder for the given resolution. Returns false and
	// disables encoding if openh264 rejects the configuration
	bool EnsureEncoderOpen(uint32 FrameWidth, uint32 FrameHeight);

	// Reads back one frame, converts it and encodes it
	void ProcessFrame(FSentryVideoFrame& Frame);

	// Tears down the openh264 encoder and resets per-encoder state
	void CloseEncoder();

	// Releases every queued frame back to its pool slot and empties the queue
	void DrainAndReleaseQueue();

	// Warns the first time a per-frame path bails out, so a silently stalled
	// pipeline is visible without flooding the log at the capture rate
	static void LogOnce(bool& bFlag, const TCHAR* Message);

	FSentrySessionReplayRecorder& Recorder;
	FSentryReplayFragmentAssembler Assembler;

	ISVCEncoder* Encoder = nullptr;
	bool bEncoderOpen = false;
	bool bResolutionChanged = false;
	FThreadSafeBool bEncodingDisabled;

	bool bLoggedFirstEncode = false;
	bool bLoggedFirstReadback = false;
	bool bLoggedSizeMismatch = false;
	bool bLoggedEncodeFailure = false;
	bool bLoggedNoReadback = false;
	bool bLoggedLockFailed = false;
	bool bLoggedFrameSkipped = false;
	bool bLoggedEmptyAccessUnit = false;

	int32 ConsecutiveEncodeFailures = 0;
	static constexpr int32 MaxConsecutiveEncodeFailures = 30;

	// Idle wait (ms) when there is no work, and recheck cadence while a frame's
	// GPU readback is still in flight
	static constexpr uint32 IdlePollIntervalMs = 50;
	static constexpr uint32 ReadbackPollIntervalMs = 2;

	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Framerate;
	int32 BitrateBps;
	float FragmentSeconds;

	// I420 scratch, reallocated only when the resolution changes
	TArray<uint8> YuvBuffer;

	// Timing (encoder-thread-only)
	double CaptureTimeBaseSeconds = -1.0;
	double LastSampleTimeSeconds = -1.0;

	FRunnableThread* Thread = nullptr;
	FEvent* WakeEvent = nullptr;
	FThreadSafeBool bStopRequested;

	FCriticalSection QueueLock;
	TArray<TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>> PendingQueue;
};

#endif // USE_SENTRY_SESSION_REPLAY && SENTRY_REPLAY_ENCODER_OPENH264
