// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

/**
 * Caches hardware and environment attributes shared by all performance metric listeners.
 *
 * Attributes cached once on construction:
 * - gpu.name, cpu.cores, ram.gb, res.x, res.y
 *
 * Attributes updated dynamically:
 * - map (on level load via PostLoadMapWithWorld delegate)
 */
class FSentryPerformanceMetricAttributes
{
public:
	FSentryPerformanceMetricAttributes();
	~FSentryPerformanceMetricAttributes();

	const TMap<FString, FSentryVariant>& GetAttributes() const;

private:
	void CacheAttributes();
	void OnMapLoaded(UWorld* World);

	TMap<FString, FSentryVariant> Attributes;
	FDelegateHandle PostLoadMapHandle;
};
