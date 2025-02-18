#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryTransactionContextAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryTransactionContextApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryTransactionContext.h"
#endif

static TSharedPtr<ISentryTransactionContext> CreateSharedSentryTransactionContext(const FString& Name, const FString& Operation)
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryTransactionContextAndroid(Name, Operation));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryTransactionContextApple(Name, Operation));
#else
	return MakeShareable(new FGenericPlatformSentryTransactionContext(Name, Operation));
#endif
}
