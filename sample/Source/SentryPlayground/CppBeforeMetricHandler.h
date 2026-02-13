// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeMetricHandler.h"

#include "CppBeforeMetricHandler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppBeforeMetricHandler : public USentryBeforeMetricHandler
{
	GENERATED_BODY()

public:
	virtual USentryMetric* HandleBeforeMetric_Implementation(USentryMetric* Metric) override;
};
