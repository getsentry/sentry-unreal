#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryScopeAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryScopeApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryScope.h"
#else
#include "Interface/SentryScopeInterface.h"
#endif

static TSharedPtr<ISentryScope> CreateSharedSentryScope()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryScopeAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryScopeApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryScope);
#else
	return nullptr;
#endif
}
