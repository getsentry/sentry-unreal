// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Misc/EngineVersionComparison.h"

class FSentryPerformanceMetricAttributes;

/**
 * Periodically samples game stats and emits them as metrics.
 *
 * Uses a ticker to fire on a configurable interval (default 60 seconds).
 *
 * Emits:
 * - game.perf.used_memory (gauge, bytes) — process physical memory usage
 * - game.perf.uobject_count (gauge) — number of active UObjects
 */
class FSentryGameStatsMonitor
{
public:
	explicit FSentryGameStatsMonitor(TSharedPtr<FSentryPerformanceMetricAttributes> InMetricAttributes);
	~FSentryGameStatsMonitor();

private:
	bool OnTick(float DeltaTime);

	TSharedPtr<FSentryPerformanceMetricAttributes> MetricAttributes;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FDelegateHandle TickerHandle;
#else
	FTSTicker::FDelegateHandle TickerHandle;
#endif
};
