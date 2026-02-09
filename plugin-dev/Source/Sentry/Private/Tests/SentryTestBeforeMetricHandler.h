// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryBeforeMetricHandler.h"
#include "SentryMetric.h"

#include "SentryTestBeforeMetricHandler.generated.h"

UCLASS()
class UTestBeforeMetricHandler : public USentryBeforeMetricHandler
{
	GENERATED_BODY()
public:
	virtual USentryMetric* HandleBeforeMetric_Implementation(USentryMetric* Metric) override
	{
		OnTestBeforeMetricHandler.ExecuteIfBound(Metric);
		return Super::HandleBeforeMetric_Implementation(Metric);
	}

	static TDelegate<void(USentryMetric*)> OnTestBeforeMetricHandler;
};
