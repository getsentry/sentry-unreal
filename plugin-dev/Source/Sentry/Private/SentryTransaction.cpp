// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryTransaction.h"

#include "Interface/SentryTransactionInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryTransactionAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryTransactionApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryTransactionDesktop.h"
#endif

USentryTransaction::USentryTransaction()
{
}

void USentryTransaction::InitWithNativeImpl(TSharedPtr<ISentryTransaction> transactionImpl)
{
	SentryTransactionNativeImpl = transactionImpl;
}

TSharedPtr<ISentryTransaction> USentryTransaction::GetNativeImpl()
{
	return SentryTransactionNativeImpl;
}
