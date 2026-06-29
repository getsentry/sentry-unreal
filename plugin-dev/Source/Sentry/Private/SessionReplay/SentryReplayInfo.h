// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryReplayInfo.generated.h"

/**
 * Description of a recorded session-replay clip.
 *
 * See https://develop.sentry.dev/sdk/telemetry/replays/ for details.
 */
USTRUCT()
struct FSentryReplayInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString ReplayId;

	UPROPERTY()
	FString VideoFilename;

	// Replay capture mode: "buffer" (rolling window) or "session" (continuous)
	UPROPERTY()
	FString ReplayType = TEXT("buffer");

	// Segment index within the replay (0 for the single crash-time buffer segment)
	UPROPERTY()
	int32 SegmentId = 0;

	UPROPERTY()
	double StartTimestampSec = 0.0;

	UPROPERTY()
	double EndTimestampSec = 0.0;

	UPROPERTY()
	int32 Width = 0;

	UPROPERTY()
	int32 Height = 0;

	UPROPERTY()
	int64 DurationMs = 0;

	UPROPERTY()
	int64 SizeBytes = 0;

	UPROPERTY()
	int32 FrameCount = 0;

	UPROPERTY()
	int32 FrameRate = 0;
};
