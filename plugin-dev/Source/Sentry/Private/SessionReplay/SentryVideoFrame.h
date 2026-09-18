// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "HAL/ThreadSafeBool.h"
#include "PixelFormat.h"
#include "RHIDefinitions.h"
#include "RHIFwd.h"
#include "Templates/UniquePtr.h"

class FRHIGPUTextureReadback;

/**
 * A capture texture shared between the render-thread capture
 * path that produces it and the encoder thread that consumes it.
 */
struct FSentryVideoFrame
{
	FTextureRHIRef Texture;

	FGPUFenceRHIRef ReadyFence;

	/**
	 * Present only for software encoders, which need the pixels in system memory.
	 *
	 * FRHIGPUTextureReadback::Lock() maps a staging surface through the immediate
	 * command list, so it is render-thread only even though IsReady() is not. The
	 * capture path therefore enqueues the copy, polls it on a later render-thread
	 * tick, and copies the result into CpuPixels before handing the frame over.
	 * The encoder thread only ever sees CpuPixels and makes no RHI calls.
	 */
	TUniquePtr<FRHIGPUTextureReadback> Readback;

	// BGRA8 pixels copied out of Readback, valid once the frame is submitted
	TArray<uint8> CpuPixels;

	// Bytes per row in CpuPixels; the staging surface pitch, not Width * 4
	int32 CpuRowPitchBytes = 0;

	// Set while a readback has been enqueued but not yet copied out
	bool bReadbackPending = false;

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
