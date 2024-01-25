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

void USentryTransactionContext::SetName(const FString& Name)
{
	if (!SentryTransactionContextNativeImpl)
		return;

	SentryTransactionContextNativeImpl->SetName(Name);
}

FString USentryTransactionContext::GetName() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetName();
}

void USentryTransactionContext::SetOrigin(const FString& Origin)
{
	if (!SentryTransactionContextNativeImpl)
		return;

	SentryTransactionContextNativeImpl->SetOrigin(Origin);
}

FString USentryTransactionContext::GetOrigin() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetOrigin();
}

void USentryTransactionContext::SetOperation(const FString& Operation)
{
	if (!SentryTransactionContextNativeImpl)
		return;

	SentryTransactionContextNativeImpl->SetOperation(Operation);
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
