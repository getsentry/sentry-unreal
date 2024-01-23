﻿// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContextApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentrySamplingContextApple::SentrySamplingContextApple(SentrySamplingContext* context)
{
	SamplingContext = context;
}

SentrySamplingContextApple::~SentrySamplingContextApple()
{
	// Put custom destructor logic here if needed
}

TMap<FString, FString> SentrySamplingContextApple::GetCustomSamplingContext() const
{
	return SentryConvertorsApple::StringMapToUnreal(SamplingContext.customSamplingContext);
}

SentrySamplingContext* SentrySamplingContextApple::GetNativeObject()
{
	return SamplingContext;
}
