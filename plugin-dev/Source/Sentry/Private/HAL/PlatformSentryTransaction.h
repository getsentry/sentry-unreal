#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryTransaction.h"
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransaction.h"
#else
#include "Interface/SentryTransactionInterface.h"
#endif

static TSharedPtr<ISentryTransaction> CreateSharedSentryTransaction()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryTransaction);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryTransactionApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryTransaction);
#else
	return nullptr;
#endif
}
