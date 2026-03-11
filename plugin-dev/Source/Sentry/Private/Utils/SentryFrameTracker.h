// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

/**
 * Tracks frame timing and emits performance metrics automatically.
 *
 * Hooks into FCoreDelegates::OnBeginFrame / OnEndFrame to measure frame duration
 * on the game thread. Emits two metrics each frame:
 * - game.frame.duration (distribution, milliseconds) — actual frame work time
 * - game.frame.slow (counter) — incremented when frame exceeds the configured threshold
 *
 * Hardware attributes (GPU, CPU cores, RAM, resolution) are cached once on start
 * and attached to every metric emission.
 */
class FSentryFrameTracker
{
public:
	FSentryFrameTracker(float InSlowFrameThresholdMs);
	~FSentryFrameTracker();

	/** Binds to frame delegates and begins tracking. */
	void Start();

	/** Unbinds delegates and stops tracking. */
	void Stop();

private:
	void OnBeginFrame();
	void OnEndFrame();

	/** Populates MetricAttributes with static hardware info. Called once during construction. */
	void CacheAttributes();

	float SlowFrameThresholdMs;
	double FrameStartTime;

	TMap<FString, FSentryVariant> MetricAttributes;

	FDelegateHandle BeginFrameHandle;
	FDelegateHandle EndFrameHandle;
};
