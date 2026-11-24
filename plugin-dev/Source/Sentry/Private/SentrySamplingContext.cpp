// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySamplingContext.h"
#include "SentryTransactionContext.h"

#include "HAL/PlatformSentrySamplingContext.h"

USentryTransactionContext* USentrySamplingContext::GetTransactionContext() const
{
	if (!NativeImpl)
		return nullptr;

	TSharedPtr<ISentryTransactionContext> transactionContextNativeImpl = NativeImpl->GetTransactionContext();

	return USentryTransactionContext::Create(transactionContextNativeImpl);
}

TMap<FString, FSentryVariant> USentrySamplingContext::GetCustomSamplingContext() const
{
	if (!NativeImpl)
		return TMap<FString, FSentryVariant>();

	return NativeImpl->GetCustomSamplingContext();
}
