// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContextAndroid.h"

#include "SentryTransactionContext.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentrySamplingContextAndroid::SentrySamplingContextAndroid(jobject samplingContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SamplingContext, samplingContext)
{
	SetupClassMethods();
}

void SentrySamplingContextAndroid::SetupClassMethods()
{
	GetTransactionContextMethod = GetMethod("getTransactionContext", "()Lio/sentry/TransactionContext;");
	GetCustomSamplingContextMethod = GetMethod("getCustomSamplingContext", "()Lio/sentry/CustomSamplingContext;");
}

USentryTransactionContext* SentrySamplingContextAndroid::GetTransactionContext() const
{
	auto transactionContext = CallObjectMethod<jobject>(GetTransactionContextMethod);
	return SentryConvertorsAndroid::SentryTransactionContextToUnreal(*transactionContext);
}

TMap<FString, FString> SentrySamplingContextAndroid::GetCustomSamplingContext() const
{
	auto customSamplingContext = CallObjectMethod<jobject>(GetCustomSamplingContextMethod);
	if(!customSamplingContext)
		return TMap<FString, FString>();

	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, *customSamplingContext);
	FSentryJavaMethod GetDataMethod = NativeCustomSamplingContext.GetMethod("getData", "()Ljava/util/Map;");

	auto data = NativeCustomSamplingContext.CallObjectMethod<jobject>(GetDataMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*data);
}
