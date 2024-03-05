// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionOptionsAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryTransactionOptionsAndroid::SentryTransactionOptionsAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::TransactionOptions, "()V")
{
	SetupClassMethods();
}

void SentryTransactionOptionsAndroid::SetupClassMethods()
{
	SetCustomSamplingContextMethod = GetMethod("setCustomSamplingContext", "(Lio/sentry/CustomSamplingContext;)V");
}

void SentryTransactionOptionsAndroid::SetCustomSamplingContext(const TMap<FString, FString>& data)
{
	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, "()V");
	FSentryJavaMethod SetMethod = NativeCustomSamplingContext.GetMethod("set", "(Ljava/lang/String;Ljava/lang/Object;)V");

	for (const auto& dataItem : data)
	{
		NativeCustomSamplingContext.CallMethod<void>(SetMethod, *GetJString(dataItem.Key), *GetJString(dataItem.Value));
	}

	CallMethod<void>(SetCustomSamplingContextMethod, NativeCustomSamplingContext.GetJObject());
}
