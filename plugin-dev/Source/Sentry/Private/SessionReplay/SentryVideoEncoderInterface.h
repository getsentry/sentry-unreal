// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "RHIDefinitions.h"
#include "Templates/SharedPointer.h"
#include "Templates/UniquePtr.h"

class FRHICommandListImmediate;
struct FSentryVideoFrame;

/** Capture and encode settings, mirrored from FSentrySessionReplayOptions. */
struct FSentryEncoderConfig
{
	uint32 Framerate = 1;
	int32 BitrateKbps = 200;
	float FragmentSeconds = 1.0f;
};

/**
 * Contract between the capture path and whichever H.264 encoder a platform has.
 *
 * The capture hook copies each presented backbuffer into a pooled BGRA8 texture
 * and hands it over; the encoder owns that frame until it calls Release() on it.
 * Encoded access units go back to the recorder as an init segment plus fMP4
 * fragments.
 *
 * Both hooks run on the render thread, so a backend can do RHI work of its own:
 * the software encoder enqueues a readback in SubmitFrame and maps it in a later
 * Poll, since FRHIGPUTextureReadback::Lock() goes through the immediate command
 * list.
 */
class ISentryEncoder
{
public:
	virtual ~ISentryEncoder() = default;

	// Spins up the encoder worker thread. Returns false if it could not start
	virtual bool StartEncoder() = 0;

	// Stops and joins the worker thread. Safe to call when not started
	virtual void StopEncoder() = 0;

	// Hands over a captured frame, on the render thread. Takes ownership of the
	// frame's pool slot until the encoder releases it
	virtual void SubmitFrame(FRHICommandListImmediate& RHICmdList, const TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>& Frame) = 0;

	// Called on the render thread every presented frame, including throttled-out
	// ones, so a backend with render-thread work pending can make progress
	// between captures
	virtual void Poll() {}

	// Flags the pool textures need beyond ShaderResource|RenderTargetable, for
	// encoders that read the frame's GPU allocation directly
	virtual ETextureCreateFlags GetFrameTextureFlags() const { return ETextureCreateFlags::None; }

	virtual uint32 GetFramerate() const = 0;
	virtual uint32 GetWidth() const = 0;
	virtual uint32 GetHeight() const = 0;

	// True once the encoder has given up, so the capture path can stop paying
	// the cost of producing frames nothing will consume
	virtual bool IsEncodingDisabled() const = 0;
};

#endif // USE_SENTRY_SESSION_REPLAY
