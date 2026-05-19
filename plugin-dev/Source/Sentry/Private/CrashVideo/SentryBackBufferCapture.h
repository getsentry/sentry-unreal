// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_CRASH_VIDEO

#include "Delegates/IDelegateInstance.h"
#include "PixelFormat.h"
#include "RHIFwd.h"

class FSentryVideoEncoder;
class SWindow;

/**
 * Hooks FSlateRenderer::OnBackBufferReadyToPresent and forwards each
 * rendered frame of the primary game window to the encoder.
 *
 * The Slate delegate fires on the render thread. We allocate a small ring
 * of pooled textures (same dimensions as the backbuffer, BGRA8 + shareable
 * flags) and issue a CopyTexture into the next slot before passing the
 * copy to the encoder. This isolates the encoder from swap-chain timing
 * and gives us a known-good shareable texture for NVENC.
 */
class FSentryBackBufferCapture
{
public:
	explicit FSentryBackBufferCapture(FSentryVideoEncoder& InEncoder);
	~FSentryBackBufferCapture();

	// Game thread. Subscribes to OnBackBufferReadyToPresent.
	bool Start();
	// Game thread. Unsubscribes.
	void Stop();

private:
	void OnBackBufferReadyToPresent_RenderThread(SWindow& SlateWindow, const FTextureRHIRef& BackBuffer);

	FTextureRHIRef AcquirePoolTexture_RenderThread(uint32 Width, uint32 Height, EPixelFormat Format);

	FSentryVideoEncoder& Encoder;
	FDelegateHandle DelegateHandle;

	// Pool of capture-destination textures (cycled round-robin).
	static constexpr int32 PoolSize = 3;
	FTextureRHIRef Pool[PoolSize];
	int32 NextPoolIndex = 0;
	uint32 PoolWidth = 0;
	uint32 PoolHeight = 0;
	EPixelFormat PoolFormat = PF_Unknown;
	bool bUnsupportedFormatLogged = false;

	// Frame throttling
	double NextCaptureTime = 0.0;
	double CapturePeriodSeconds = 1.0 / 30.0;
};

#endif // USE_SENTRY_CRASH_VIDEO
