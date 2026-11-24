// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryDataTypes.h"

#include "SentryBeforeMetricHandler.generated.h"

class USentryMetric;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeMetricHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryMetric* HandleBeforeMetric(USentryMetric* Metric);
	virtual USentryMetric* HandleBeforeMetric_Implementation(USentryMetric* Metric);
};
