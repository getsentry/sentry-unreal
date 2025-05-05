#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryTransactionContext.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryTransactionContext.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransactionContext.h"
#else
#include "Interface/SentryTransactionContextInterface.h"
#endif

static TSharedPtr<ISentryTransactionContext> CreateSharedSentryTransactionContext(const FString& Name, const FString& Operation)
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryTransactionContext(Name, Operation));
#elif PLATFORM_APPLE
	return MakeShareable(new FAppleSentryTransactionContext(Name, Operation));
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryTransactionContext(Name, Operation));
#else
	return nullptr;
#endif
}
