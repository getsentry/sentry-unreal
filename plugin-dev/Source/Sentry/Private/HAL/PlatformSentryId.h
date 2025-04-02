#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryIdAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryIdApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryId.h"
#else
#include "Interface/SentryIdInterface.h"
#endif

static TSharedPtr<ISentryId> CreateSharedSentryId(const FString& Id)
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryIdAndroid(Id));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryIdApple(Id));
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryId(Id));
#else
	return nullptr;
#endif
}
