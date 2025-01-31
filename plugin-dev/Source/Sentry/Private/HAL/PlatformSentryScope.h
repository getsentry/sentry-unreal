#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryScopeAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryScopeApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryScope.h"
#endif

static TSharedPtr<ISentryScope> CreateSharedSentryScope()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentryScopeAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentryScopeApple);
#else
    return MakeShareable(new FGenericPlatformSentryScope);
#endif
}
