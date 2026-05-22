// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_SESSION_REPLAY

#include "Delegates/IDelegateInstance.h"
#include "PixelFormat.h"
#include "RHIFwd.h"

class FSentryVideoEncoder;
class SWindow;

/**
 * Hooks FSlateRenderer::OnBackBufferReadyToPresent and forwards each
 * rendered frame of the primary game window to the encoder.
 *
 * Per frame on the render thread:
 *   1. Hardware-copy the backbuffer into a "scratch" texture that has the
 *      same format but also carries ShaderResource flag (Slate backbuffers
 *      don't, which makes them unusable as an SRV directly).
 *   2. AddDrawTexturePass(scratch -> pool[BGRA8]). When the scratch format
 *      matches BGRA8 this stays a hardware copy; otherwise the engine's
 *      built-in pixel-shader path converts HDR/10-bit to BGRA8.
 * The pool slot is then handed to NVENC.
 */
class FSentryBackBufferCapture
{
public:
	explicit FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder);
	~FSentryBackBufferCapture();

	// Subscribes to OnBackBufferReadyToPresent (game thread).
	bool Start();
	// Unsubscribes from OnBackBufferReadyToPresent (game thread).
	void Stop();

private:
	void OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer);

	FTextureRHIRef AcquirePoolTexture_RenderThread(uint32 Width, uint32 Height);
	FTextureRHIRef AcquireScratchTexture_RenderThread(uint32 Width, uint32 Height, EPixelFormat Format);

	FSentryVideoEncoder& Encoder;
	FDelegateHandle DelegateHandle;

	// Pool of capture-destination textures (cycled round-robin). Always
	// PF_B8G8R8A8 because that's what NVENC D3D12 requires; source-format
	// conversion happens inside AddDrawTexturePass.
	static constexpr int32 PoolSize = 3;
	FTextureRHIRef Pool[PoolSize];
	int32 NextPoolIndex = 0;
	uint32 PoolWidth = 0;
	uint32 PoolHeight = 0;

	// Scratch texture matching the backbuffer's format with ShaderResource
	// flag added — the Slate backbuffer doesn't carry that flag so
	// AddDrawTexturePass' shader fallback can't bind it as an SRV directly.
	FTextureRHIRef ScratchTexture;
	uint32 ScratchWidth = 0;
	uint32 ScratchHeight = 0;
	EPixelFormat ScratchFormat = PF_Unknown;

	// Frame throttling
	double NextCaptureTime = 0.0;
	double CapturePeriodSeconds = 1.0 / 30.0;
};

#endif // USE_SENTRY_SESSION_REPLAY
