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
	GetCustomSamplingContextMethod = GetMethod("getCustomSamplingContext", "()Lio/sentry/CustomSamplingContext;");
}

TMap<FString, FString> SentrySamplingContextAndroid::GetCustomSamplingContext() const
{
	auto customSamplingContext = CallObjectMethod<jobject>(GetCustomSamplingContextMethod);
	if(!customSamplingContext)
		return TMap<FString, FString>();

	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, customSamplingContext);
	FSentryJavaMethod GetDataMethod = NativeCustomSamplingContext.GetMethod("getData", "()Ljava/util/Map;");

	auto data = NativeCustomSamplingContext.CallMethod<jobject>(GetDataMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}
