#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryIdAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryIdApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryId.h"
#endif

static TSharedPtr<ISentryId> CreateSharedSentryId()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentryIdAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentryIdApple);
#else
    return MakeShareable(new FGenericPlatformSentryId);
#endif
}
