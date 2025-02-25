#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryTransactionAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransaction.h"
#endif

static TSharedPtr<ISentryTransaction> CreateSharedSentryTransaction()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentryTransactionAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentryTransactionApple);
#elif USE_SENTRY_NATIVE
    return MakeShareable(new FGenericPlatformSentryTransaction);
#else
	return nullptr;
#endif
}
