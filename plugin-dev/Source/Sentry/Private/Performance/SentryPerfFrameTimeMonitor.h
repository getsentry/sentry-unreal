// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "ChartCreation.h"
#include "CoreMinimal.h"

class FSentryPerfMetricAttributes;

/**
 * Consumes per-frame performance data from the engine's FPerformanceTrackingSystem.
 *
 * Registered via GEngine->AddPerformanceDataConsumer(). Emits sampled metrics:
 * - game.perf.frame_time (distribution, milliseconds) — frame time from engine tracking
 * - game.perf.fps (gauge) — engine-smoothed average FPS
 * - game.perf.game_thread (distribution, milliseconds) — game thread work time
 * - game.perf.render_thread (distribution, milliseconds) — render thread work time
 * - game.perf.gpu (distribution, milliseconds) — GPU frame time
 */
class FSentryPerfFrameTimeMonitor : public IPerformanceDataConsumer
{
public:
	explicit FSentryPerfFrameTimeMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes);

	// IPerformanceDataConsumer
	virtual void StartCharting() override;
	virtual void ProcessFrame(const FFrameData& FrameData) override;
	virtual void StopCharting() override;

private:
	int32 SampleInterval;
	uint64 FrameCount;
	TSharedPtr<FSentryPerfMetricAttributes> MetricAttributes;
};
