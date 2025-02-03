#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryTransactionAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryTransaction.h"
#endif

static TSharedPtr<ISentryTransaction> CreateSharedSentryTransaction()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentryTransactionAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentryTransactionApple);
#else
    return MakeShareable(new FGenericPlatformSentryTransaction);
#endif
}
