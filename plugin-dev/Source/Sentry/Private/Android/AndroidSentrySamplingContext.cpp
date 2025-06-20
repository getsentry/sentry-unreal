// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentrySamplingContext.h"
#include "AndroidSentryTransactionContext.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentrySamplingContext::FAndroidSentrySamplingContext(jobject samplingContext)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SamplingContext, samplingContext)
{
	SetupClassMethods();
}

void FAndroidSentrySamplingContext::SetupClassMethods()
{
	GetTransactionContextMethod = GetMethod("getTransactionContext", "()Lio/sentry/TransactionContext;");
	GetCustomSamplingContextMethod = GetMethod("getCustomSamplingContext", "()Lio/sentry/CustomSamplingContext;");
}

TSharedPtr<ISentryTransactionContext> FAndroidSentrySamplingContext::GetTransactionContext() const
{
	auto transactionContext = CallObjectMethod<jobject>(GetTransactionContextMethod);
	return MakeShareable(new FAndroidSentryTransactionContext(*transactionContext));
}

TMap<FString, FString> FAndroidSentrySamplingContext::GetCustomSamplingContext() const
{
	auto customSamplingContext = CallObjectMethod<jobject>(GetCustomSamplingContextMethod);
	if (!customSamplingContext)
		return TMap<FString, FString>();

	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, *customSamplingContext);
	FSentryJavaMethod GetDataMethod = NativeCustomSamplingContext.GetMethod("getData", "()Ljava/util/Map;");

	auto data = NativeCustomSamplingContext.CallObjectMethod<jobject>(GetDataMethod);
	return FAndroidSentryConverters::StringMapToUnreal(*data);
}
