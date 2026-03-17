// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "ChartCreation.h"
#include "CoreMinimal.h"
#include "SentryVariant.h"

/**
 * Consumes per-frame performance data from the engine's FPerformanceTrackingSystem.
 *
 * Registered via GEngine->AddPerformanceDataConsumer(). Emits sampled metrics:
 * - game.perf.duration (distribution, milliseconds) — frame time from engine tracking
 * - game.perf.fps (gauge) — engine-smoothed average FPS
 * - game.perf.game_thread (distribution, milliseconds) — game thread work time
 * - game.perf.render_thread (distribution, milliseconds) — render thread work time
 * - game.perf.gpu (distribution, milliseconds) — GPU frame time
 *
 * Hardware attributes (GPU, CPU cores, RAM, resolution) are cached once on construction.
 * Map attribute is updated dynamically on level load.
 */
class FSentryPerformanceConsumer : public IPerformanceDataConsumer
{
public:
	FSentryPerformanceConsumer();
	~FSentryPerformanceConsumer();

	// IPerformanceDataConsumer
	virtual void StartCharting() override;
	virtual void ProcessFrame(const FFrameData& FrameData) override;
	virtual void StopCharting() override;

private:
	void CacheAttributes();
	void OnMapLoaded(UWorld* World);

	int32 SampleInterval;
	uint64 FrameCount;
	TMap<FString, FSentryVariant> MetricAttributes;

	FDelegateHandle PostLoadMapHandle;
};
