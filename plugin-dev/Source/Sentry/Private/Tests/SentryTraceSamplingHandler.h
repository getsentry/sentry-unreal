// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryTraceSampler.h"
#include "SentrySamplingContext.h"

#include "SentryTraceSamplingHandler.generated.h"

UCLASS()
class UTraceSamplingTestHandler : public USentryTraceSampler
{
	GENERATED_BODY()
public:
	virtual bool Sample_Implementation(USentrySamplingContext* samplingContext, float& samplingValue) override
	{
		OnTraceSamplingTestHandler.ExecuteIfBound(samplingContext);
		return Super::Sample_Implementation(samplingContext, samplingValue);
	}

	static TDelegate<void(USentrySamplingContext*)> OnTraceSamplingTestHandler;
};
