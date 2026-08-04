// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Delegates/IDelegateInstance.h"
#include "Misc/EngineVersionComparison.h"
#include "PixelFormat.h"
#include "RHIAccess.h"
#include "RHIDefinitions.h"
#include "RHIFwd.h"
#include "Templates/SharedPointer.h"

struct FSentryVideoFrame;
class FSentryVideoEncoder;
class ISlateViewportProvider;
class SWindow;

/**
 * Hooks FSlateRenderer::OnBackBufferReadyToPresent and forwards each rendered
 * frame of the primary game window to the encoder.
 *
 * Per frame on the render thread:
 *   1. Hardware-copy the backbuffer into a "scratch" texture that has the same
 *      format but also carries the ShaderResource flag (Slate backbuffers don't,
 *      which makes them unusable as an SRV directly).
 *   2. AddDrawTexturePass scratch -> a BGRA8 destination. When the scratch
 *      format is already BGRA8 this stays a hardware copy; otherwise the
 *      engine's built-in pixel-shader path converts HDR/10-bit to BGRA8.
 *      Destination is the encoder pool slot on Windows; on Apple platforms it's the
 *      "converted" texture because Metal forbids RenderTargetable | CPUReadback.
 *   3. Apple only: hardware-copy converted -> encoder pool slot (CPUReadback BGRA8).
 * The pool slot is then handed to the encoder.
 */
class FSentryBackBufferCapture
{
public:
	explicit FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder);
	~FSentryBackBufferCapture();

	// Subscribes to OnBackBufferReadyToPresent (game thread)
	bool Start();
	// Unsubscribes from OnBackBufferReadyToPresent (game thread)
	void Stop();

private:
	// Single texture tracked alongside the config it was created with, so the
	// acquire helper can detect changes and recreate
	struct FCachedTexture
	{
		FTextureRHIRef Texture;
		uint32 Width = 0;
		uint32 Height = 0;
		EPixelFormat Format = PF_Unknown;
		ETextureCreateFlags Flags = ETextureCreateFlags::None;
	};

#if UE_VERSION_OLDER_THAN(5, 8, 0)
	void OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer);
#else
	void OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, ISlateViewportProvider& ViewportProvider);
#endif

	// Returns true if SlateWindow is a real top-level window whose backbuffer should
	// be captured. Transient overlays (tooltips, popup menus, notification toasts,
	// cursor decorators) are rejected. Render thread only.
	bool AcceptWindow_RenderThread(const SWindow& SlateWindow);

	// Captures a single backbuffer frame and forwards it to the encoder (render thread)
	void CaptureBackBuffer_RenderThread(const FTextureRHIRef& BackBuffer);

	// Returns the cached texture, recreated when any of (Width, Height, Format,
	// Flags) differs from the previous call. Returns null on creation failure
	static FTextureRHIRef AcquireCachedTexture_RenderThread(FCachedTexture& Cache, uint32 Width, uint32 Height, EPixelFormat Format,
		ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName);

	// Acquires a free pool frame and ensures its texture exists (recreated on config
	// change). Marks the acquired frame in flight. Returns null when every slot is
	// still owned by the encoder or on creation failure
	TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe> AcquireFrame_RenderThread(uint32 Width, uint32 Height, EPixelFormat Format,
		ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName);

	FSentryVideoEncoder& Encoder;

	FDelegateHandle BackBufferReadyHandle;

	// SRV-able copy of the backbuffer at its source format. Slate backbuffers
	// don't carry the SRV flag, so they can't be sampled in a shader directly
	FCachedTexture Scratch;

	// BGRA8 RenderTargetable texture. Used on Apple platforms as the draw pass output
	// before the final hardware copy into the CPUReadback EncoderPool slot
	FCachedTexture Converted;

	// Number of encoder frames pooled
	static constexpr int32 FramePoolSize = 5;

	// Pool of frames submitted to the encoder. Each frame stays in flight from
	// acquire until the encoder releases it, so the render thread never overwrites
	// a slot the encoder still holds
	TArray<TSharedPtr<FSentryVideoFrame, ESPMode::ThreadSafe>> EncoderPool;

	// Frame throttling
	double NextCaptureTime = 0.0;
	double CapturePeriodSeconds = 1.0 / 30.0;
};

#endif // USE_SENTRY_SESSION_REPLAY
