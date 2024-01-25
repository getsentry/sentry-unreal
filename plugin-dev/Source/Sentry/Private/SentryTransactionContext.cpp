// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContext.h"

#include "Interface/SentryTransactionInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryTransactionContextAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryTransactionContextApple.h"
#endif

USentryTransactionContext::USentryTransactionContext()
{
}

FString USentryTransactionContext::GetName() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetName();
}

FString USentryTransactionContext::GetOrigin() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetOrigin();
}

FString USentryTransactionContext::GetOperation() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetOperation();
}

void USentryTransactionContext::InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl)
{
	SentryTransactionContextNativeImpl = transactionContextImpl;
}

TSharedPtr<ISentryTransactionContext> USentryTransactionContext::GetNativeImpl()
{
	return SentryTransactionContextNativeImpl;
}
