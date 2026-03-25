// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

class FSentryPerfMetricAttributes;

/**
 * Periodically samples game stats and emits them as metrics.
 *
 * Uses a ticker to fire on a configurable interval (default 60 seconds).
 *
 * Emits:
 * - game.perf.used_memory (gauge, bytes) — process physical memory usage
 * - game.perf.uobject_count (gauge) — number of active UObjects (UE 5.3+)
 */
class FSentryPerfGameStatsMonitor
{
public:
	explicit FSentryPerfGameStatsMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes);
	~FSentryPerfGameStatsMonitor();

private:
	bool OnTick(float DeltaTime);

	TSharedPtr<FSentryPerfMetricAttributes> MetricAttributes;

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FDelegateHandle TickerHandle;
#else
	FTSTicker::FDelegateHandle TickerHandle;
#endif
};
