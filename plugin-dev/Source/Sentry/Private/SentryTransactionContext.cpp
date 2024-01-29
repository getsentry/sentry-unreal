// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryTransactionContext.h"

#include "Interface/SentryTransactionContextInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryTransactionContextAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryTransactionContextApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryTransactionContextDesktop.h"
#endif

USentryTransactionContext::USentryTransactionContext()
{
}

void USentryTransactionContext::Initialize(const FString& Name, const FString& Operation)
{
#if PLATFORM_ANDROID
	SentryTransactionContextNativeImpl = MakeShareable(new SentryTransactionContextAndroid(Name, Operation));
#elif PLATFORM_IOS || PLATFORM_MAC
	SentryTransactionContextNativeImpl = MakeShareable(new SentryTransactionContextApple(Name, Operation));
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
	SentryTransactionContextNativeImpl = MakeShareable(new SentryTransactionContextDesktop(Name, Operation));
#endif
}

FString USentryTransactionContext::GetName() const
{
	if (!SentryTransactionContextNativeImpl)
		return FString();

	return SentryTransactionContextNativeImpl->GetName();
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
