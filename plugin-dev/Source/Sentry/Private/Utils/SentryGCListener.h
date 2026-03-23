// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

/**
 * Listens for garbage collection events and emits GC pause duration as a metric.
 *
 * Measures the blocking GC pause between GetGarbageCollectStartedDelegate (GC lock held)
 * and GetPostGarbageCollect (lock about to be released). This captures the actual
 * game-thread-blocking pause that impacts gameplay.
 *
 * Emits: game.perf.gc_time (distribution, milliseconds)
 *
 * Hardware attributes (GPU, CPU cores, RAM, resolution) are cached once on construction.
 * Map attribute is updated dynamically on level load.
 */
class FSentryGCListener
{
public:
	FSentryGCListener();
	~FSentryGCListener();

private:
	void OnGCStarted();
	void OnPostGC();
	void CacheAttributes();
	void OnMapLoaded(UWorld* World);

	double GCStartTime;

	TMap<FString, FSentryVariant> MetricAttributes;

	FDelegateHandle GCStartedHandle;
	FDelegateHandle PostGCHandle;
	FDelegateHandle PostLoadMapHandle;
};
