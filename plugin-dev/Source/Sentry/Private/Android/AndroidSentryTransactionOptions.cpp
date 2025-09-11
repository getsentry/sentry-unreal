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
	SetBindToScopeMethod = GetMethod("setBindToScope", "(Z)V");
}

void FAndroidSentryTransactionOptions::SetCustomSamplingContext(const TMap<FString, FSentryVariant>& data)
{
	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, "()V");
	FSentryJavaMethod SetMethod = NativeCustomSamplingContext.GetMethod("set", "(Ljava/lang/String;Ljava/lang/Object;)V");

	for (const auto& dataItem : data)
	{
		NativeCustomSamplingContext.CallMethod<void>(SetMethod, *GetJString(dataItem.Key), FAndroidSentryConverters::VariantToNative(dataItem.Value)->GetJObject());
	}

	CallMethod<void>(SetCustomSamplingContextMethod, NativeCustomSamplingContext.GetJObject());
}

void FAndroidSentryTransactionOptions::SetBindToScope(bool bindToScope)
{
	CallMethod<void>(SetBindToScopeMethod, bindToScope);
}
