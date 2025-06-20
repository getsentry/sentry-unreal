// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryTraceSampler.h"
#include "SentrySamplingContext.h"

bool USentryTraceSampler::Sample_Implementation(USentrySamplingContext* samplingContext, float& samplingValue)
{
	samplingValue = 1.0f;
	return true;
}
