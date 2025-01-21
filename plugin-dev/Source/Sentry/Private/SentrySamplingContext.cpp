// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContext.h"
#include "SentryTransactionContext.h"

#include "Interface/SentrySamplingContextInterface.h"

USentryTransactionContext* USentrySamplingContext::GetTransactionContext() const
{
	if (!NativeImpl)
		return nullptr;

	TSharedPtr<ISentryTransactionContext> transactionContextNativeImpl = NativeImpl->GetTransactionContext();

	return USentryTransactionContext::Create(transactionContextNativeImpl);
}

TMap<FString, FString> USentrySamplingContext::GetCustomSamplingContext() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetCustomSamplingContext();
}
