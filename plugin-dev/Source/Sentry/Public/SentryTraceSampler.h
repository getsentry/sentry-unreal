// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTraceSampler.generated.h"

class USentrySamplingContext;

UCLASS(Blueprintable)
class SENTRY_API USentryTraceSampler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	bool Sample(USentrySamplingContext* samplingContext, float& samplingValue);
	virtual bool Sample_Implementation(USentrySamplingContext* samplingContext, float& samplingValue);
};
