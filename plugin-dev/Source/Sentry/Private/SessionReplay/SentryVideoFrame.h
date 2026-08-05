// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "HAL/ThreadSafeBool.h"
#include "PixelFormat.h"
#include "RHIDefinitions.h"
#include "RHIFwd.h"

/**
 * A capture texture shared between the render-thread capture
 * path that produces it and the encoder thread that consumes it.
 */
struct FSentryVideoFrame
{
	FTextureRHIRef Texture;

	FGPUFenceRHIRef ReadyFence;

	double CaptureTimeSeconds = 0.0;

	uint32 Width = 0;
	uint32 Height = 0;
	EPixelFormat Format = PF_Unknown;
	ETextureCreateFlags Flags = ETextureCreateFlags::None;

	// Called by the capture path to claim this frame before writing to it. Marks
	// the frame in use and returns true, or returns false if the encoder has not
	// released it yet
	bool TryAcquire();

	// Called by the encoder to mark this frame free again, once it has finished
	// encoding it, so the capture path can reuse it
	void Release();

	// Checks if GPU has finished writing Texture (ReadyFence signalled), so the
	// encoder may hand it to the hardware encoder. A frame with no fence counts as
	// ready
	bool IsGpuWriteComplete() const;

private:
	FThreadSafeBool bInFlight;
};

#endif // USE_SENTRY_SESSION_REPLAY
