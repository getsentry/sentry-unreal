#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryTransaction.h"
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransaction.h"
#else
#include "Null/NullSentryTransaction.h"
#endif

static TSharedPtr<ISentryTransaction> CreateSharedSentryTransaction()
{
	return MakeShareable(new FPlatformSentryTransaction);
}
