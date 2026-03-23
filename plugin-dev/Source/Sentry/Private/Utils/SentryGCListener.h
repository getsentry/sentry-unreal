// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FSentryPerformanceMetricAttributes;

/**
 * Listens for garbage collection events and emits GC pause duration as a metric.
 *
 * Measures the blocking GC pause between GetGarbageCollectStartedDelegate (GC lock held)
 * and GetPostGarbageCollect (lock about to be released). This captures the actual
 * game-thread-blocking pause that impacts gameplay.
 *
 * Emits: game.perf.gc_time (distribution, milliseconds)
 */
class FSentryGCListener
{
public:
	explicit FSentryGCListener(TSharedPtr<FSentryPerformanceMetricAttributes> InMetricAttributes);
	~FSentryGCListener();

private:
	void OnGCStarted();
	void OnPostGC();

	double GCStartTime;

	TSharedPtr<FSentryPerformanceMetricAttributes> MetricAttributes;

	FDelegateHandle GCStartedHandle;
	FDelegateHandle PostGCHandle;
};
