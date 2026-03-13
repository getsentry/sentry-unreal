// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

/**
 * Tracks frame timing and emits performance metrics automatically.
 *
 * Hooks into FCoreDelegates::OnBeginFrame / OnEndFrame to measure frame duration
 * on the game thread. Emits every Nth frame (configurable sample interval):
 * - game.frame.duration (distribution, milliseconds) — actual frame work time
 * - game.fps (gauge) — engine-smoothed average FPS (from GAverageFPS)
 *
 * Hardware attributes (GPU, CPU cores, RAM, resolution) are cached once on start
 * and attached to every metric emission.
 */
class FSentryFrameTracker
{
public:
	FSentryFrameTracker(int32 InSampleInterval);
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

	int32 SampleInterval;
	uint64 FrameCount;
	double FrameStartTime;

	TMap<FString, FSentryVariant> MetricAttributes;

	FDelegateHandle BeginFrameHandle;
	FDelegateHandle EndFrameHandle;
};
