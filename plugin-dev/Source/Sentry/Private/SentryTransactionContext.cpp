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

void USentryTransactionContext::InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl)
{
	SentryTransactionNativeImpl = transactionContextImpl;
}

TSharedPtr<ISentryTransactionContext> USentryTransactionContext::GetNativeImpl()
{
	return SentryTransactionNativeImpl;
}
