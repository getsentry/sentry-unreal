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

static TSharedPtr<ISentryId> CreateSharedSentryId()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentryIdAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentryIdApple);
#elif USE_SENTRY_NATIVE
    return MakeShareable(new FGenericPlatformSentryId);
#else
	return nullptr;
#endif
}
