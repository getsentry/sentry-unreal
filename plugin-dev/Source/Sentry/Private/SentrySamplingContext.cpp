// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContext.h"

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

	return SentrySamplingContextNativeImpl->GetTransactionContext();
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
