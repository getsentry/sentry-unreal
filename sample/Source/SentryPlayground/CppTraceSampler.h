// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryTraceSampler.h"

#include "CppTraceSampler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppTraceSampler : public USentryTraceSampler
{
	GENERATED_BODY()

public:
	virtual bool Sample_Implementation(USentrySamplingContext* samplingContext, float& samplingValue) override;
};
