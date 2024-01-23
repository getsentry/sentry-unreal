// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContextAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentrySamplingContextAndroid::SentrySamplingContextAndroid(jobject samplingContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SamplingContext, samplingContext)
{
	SetupClassMethods();
}

void SentrySamplingContextAndroid::SetupClassMethods()
{
}

TMap<FString, FString> SentrySamplingContextAndroid::GetCustomSamplingContext() const
{
	return TMap<FString, FString>();
}
