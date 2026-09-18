// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

struct FSentryVideoFrame;
class FSentrySessionReplayRecorder;

/**
 * Contract between the capture path and whichever H.264 encoder a platform has.
 *
 * Implementations run their own worker thread, consume frames handed over by
 * FSentryBackBufferCapture, and publish an init segment plus complete fMP4
 * fragments back to the recorder via FSentrySessionReplayRecorder::OnInitSegmentReady
 * and ::OnFragmentReady.
 *
 * Two backends exist:
 *   - FSentryVideoEncoder      hardware H.264 via the engine's AVCodecs plugin
 *                              (Windows, Linux, Mac, iOS)
 *   - FSentryOpenH264Encoder   software H.264 via openh264, for platforms that
 *                              expose no encoder at all (Xbox)
 */
class ISentryVideoEncoder
{
public:
	virtual ~ISentryVideoEncoder() = default;

	// Spins up the encoder worker thread. Returns false if it could not start
	virtual bool StartEncoder() = 0;

	// Stops and joins the worker thread. Safe to call when not started
	virtual void StopEncoder() = 0;

	// Hands a captured frame to the encoder. Takes ownership of the frame's pool
	// slot until the encoder releases it. Called from the render thread
	virtual void SubmitFrame(const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& Frame) = 0;

	virtual uint32 GetFramerate() const = 0;
	virtual uint32 GetWidth() const = 0;
	virtual uint32 GetHeight() const = 0;

	// True once the encoder has given up, so the capture path can stop paying
	// the cost of producing frames nothing will consume
	virtual bool IsEncodingDisabled() const = 0;

	/**
	 * Whether frames must be readable by the CPU.
	 *
	 * Hardware encoders take the GPU resource directly, so the capture path can
	 * hand over a plain render-target texture. Software encoders need the pixels
	 * in system memory, which means a CPUReadback-flagged pool slot and an extra
	 * copy, since a texture cannot be both render-target and readback.
	 */
	virtual bool RequiresCpuReadableFrames() const = 0;
};

/**
 * Creates the encoder backend this platform was built with, or null when the
 * build has no encoder available.
 */
TUniquePtr<ISentryVideoEncoder> CreateSentryVideoEncoder(FSentrySessionReplayRecorder& Recorder,
	uint32 Framerate, int32 BitrateKbps, float FragmentSeconds, int32 MaxCaptureHeight);

#endif // USE_SENTRY_SESSION_REPLAY
