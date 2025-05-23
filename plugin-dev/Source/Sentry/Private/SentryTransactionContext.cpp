// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryTransactionContext.h"

#include "HAL/PlatformSentryTransactionContext.h"

void USentryTransactionContext::Initialize(const FString& Name, const FString& Operation)
{
	NativeImpl = CreateSharedSentryTransactionContext(Name, Operation);
}

FString USentryTransactionContext::GetName() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetName();
}

FString USentryTransactionContext::GetOperation() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetOperation();
}
