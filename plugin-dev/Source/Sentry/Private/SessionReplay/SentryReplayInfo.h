// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Description of a recorded session-replay clip.
 *
 * See https://develop.sentry.dev/sdk/telemetry/replays/ for details.
 */
struct FSentryReplayInfo
{
	FString ReplayId;

	// Segment index within the replay (0 for the single crash-time buffer segment)
	int32 SegmentId = 0;

	// Replay capture mode: "buffer" (rolling window) or "session" (continuous)
	FString ReplayType = TEXT("buffer");

	double StartTimestampSec = 0.0;
	double EndTimestampSec = 0.0;

	FString VideoPath;

	int32 Width = 0;
	int32 Height = 0;

	int64 DurationMs = 0;
	int64 SizeBytes = 0;
	int32 FrameCount = 0;
	int32 FrameRate = 0;
};
