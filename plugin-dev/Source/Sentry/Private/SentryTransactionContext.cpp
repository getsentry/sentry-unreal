// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContext.h"

#include "Interface/SentryTransactionContextInterface.h"

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
