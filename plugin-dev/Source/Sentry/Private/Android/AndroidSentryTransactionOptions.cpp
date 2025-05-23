// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryTransactionOptions.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryTransactionOptions::FAndroidSentryTransactionOptions()
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionOptions, "()V")
{
	SetupClassMethods();
}

void FAndroidSentryTransactionOptions::SetupClassMethods()
{
	SetCustomSamplingContextMethod = GetMethod("setCustomSamplingContext", "(Lio/sentry/CustomSamplingContext;)V");
}

void FAndroidSentryTransactionOptions::SetCustomSamplingContext(const TMap<FString, FString>& data)
{
	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, "()V");
	FSentryJavaMethod SetMethod = NativeCustomSamplingContext.GetMethod("set", "(Ljava/lang/String;Ljava/lang/Object;)V");

	for (const auto& dataItem : data)
	{
		NativeCustomSamplingContext.CallMethod<void>(SetMethod, *GetJString(dataItem.Key), *GetJString(dataItem.Value));
	}

	CallMethod<void>(SetCustomSamplingContextMethod, NativeCustomSamplingContext.GetJObject());
}
