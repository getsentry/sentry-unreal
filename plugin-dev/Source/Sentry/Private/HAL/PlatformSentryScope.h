#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryScope.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryScope.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryScope.h"
#else
#include "Interface/SentryScopeInterface.h"
#endif

static TSharedPtr<ISentryScope> CreateSharedSentryScope()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryScope);
#elif PLATFORM_APPLE
	return MakeShareable(new FAppleSentryScope);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryScope);
#else
	return nullptr;
#endif
}
