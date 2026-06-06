// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Delegates/IDelegateInstance.h"
#include "PixelFormat.h"
#include "RHIAccess.h"
#include "RHIDefinitions.h"
#include "RHIFwd.h"

class FSentryVideoEncoder;
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
 *      Destination is the encoder pool slot on Windows; on Mac it's the
 *      "converted" texture because Metal forbids RenderTargetable | CPUReadback.
 *   3. Mac only: hardware-copy converted -> encoder pool slot (CPUReadback BGRA8).
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

	// N-slot pool sharing one config across all slots. Slots are created lazily
	// and recycled when their refcount drops back to 1 (no other holder)
	struct FCachedTexturePool
	{
		TArray<FTextureRHIRef> Slots;
		uint32 Width = 0;
		uint32 Height = 0;
		EPixelFormat Format = PF_Unknown;
		ETextureCreateFlags Flags = ETextureCreateFlags::None;
	};

	void OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer);

	// Returns the cached texture, recreated when any of (Width, Height, Format,
	// Flags) differs from the previous call. Returns null on creation failure
	static FTextureRHIRef AcquireCachedTexture_RenderThread(FCachedTexture& Cache, uint32 Width, uint32 Height, EPixelFormat Format,
		ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName);

	// Returns a texture pool slot whose refcount is <= 1, recreating the entire pool when
	// the config changes. Returns null when every slot is still in flight
	static FTextureRHIRef AcquireTexturePoolSlot_RenderThread(FCachedTexturePool& Pool, uint32 Width, uint32 Height, EPixelFormat Format,
		ETextureCreateFlags Flags, ERHIAccess InitialState, const TCHAR* DebugName);

	FSentryVideoEncoder& Encoder;

	FDelegateHandle BackBufferReadyHandle;

	// SRV-able copy of the backbuffer at its source format. Slate backbuffers
	// don't carry the SRV flag, so they can't be sampled in a shader directly
	FCachedTexture Scratch;

	// BGRA8 RenderTargetable texture. Used on Mac as the draw pass output
	// before the final hardware copy into the CPUReadback EncoderPool slot
	FCachedTexture Converted;

	// Pool of textures that are submitted to the encoder. Ref-counted because
	// the encoder thread holds these across frames
	FCachedTexturePool EncoderPool;

	// Frame throttling
	double NextCaptureTime = 0.0;
	double CapturePeriodSeconds = 1.0 / 30.0;
};

#endif // USE_SENTRY_SESSION_REPLAY
