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
	SetStartTimestampMethod = GetMethod("setStartTimestamp", "(Lio/sentry/SentryDate;)V");
}

void FAndroidSentryTransactionOptions::SetCustomSamplingContext(const TMap<FString, FSentryVariant>& data)
{
	FSentryJavaObjectWrapper NativeCustomSamplingContext(SentryJavaClasses::CustomSamplingContext, "()V");
	FSentryJavaMethod SetMethod = NativeCustomSamplingContext.GetMethod("set", "(Ljava/lang/String;Ljava/lang/Object;)V");

	for (const auto& dataItem : data)
	{
		TSharedPtr<FSentryJavaObjectWrapper> nativeValue = FAndroidSentryConverters::VariantToNative(dataItem.Value);
		if (!nativeValue.IsValid())
		{
			continue;
		}

		NativeCustomSamplingContext.CallMethod<void>(SetMethod, *GetJString(dataItem.Key), nativeValue->GetJObject());
	}

	CallMethod<void>(SetCustomSamplingContextMethod, NativeCustomSamplingContext.GetJObject());
}

void FAndroidSentryTransactionOptions::SetBindToScope(bool bindToScope)
{
	CallMethod<void>(SetBindToScopeMethod, bindToScope);
}

void FAndroidSentryTransactionOptions::SetStartTimestamp(int64 timestamp)
{
	CallMethod<void>(SetStartTimestampMethod, FAndroidSentryConverters::TimestampToNative(timestamp)->GetJObject());
}
