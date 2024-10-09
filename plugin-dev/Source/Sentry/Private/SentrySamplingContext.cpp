// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContext.h"
#include "SentryTransactionContext.h"

#include "Interface/SentrySamplingContextInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentrySamplingContextAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySamplingContextApple.h"
#endif

USentrySamplingContext::USentrySamplingContext()
{
}

USentryTransactionContext* USentrySamplingContext::GetTransactionContext() const
{
	if (!SentrySamplingContextNativeImpl)
		return nullptr;

	TSharedPtr<ISentryTransactionContext> transactionContextNativeImpl = SentrySamplingContextNativeImpl->GetTransactionContext();

	USentryTransactionContext* unrealTransactionContext = NewObject<USentryTransactionContext>();
	unrealTransactionContext->InitWithNativeImpl(transactionContextNativeImpl);

	return unrealTransactionContext;
}

TMap<FString, FString> USentrySamplingContext::GetCustomSamplingContext() const
{
	if (!SentrySamplingContextNativeImpl)
		return TMap<FString, FString>();

	return SentrySamplingContextNativeImpl->GetCustomSamplingContext();
}

void USentrySamplingContext::InitWithNativeImpl(TSharedPtr<ISentrySamplingContext> samplingContextImpl)
{
	SentrySamplingContextNativeImpl = samplingContextImpl;
}

TSharedPtr<ISentrySamplingContext> USentrySamplingContext::GetNativeImpl()
{
	return SentrySamplingContextNativeImpl;
}
