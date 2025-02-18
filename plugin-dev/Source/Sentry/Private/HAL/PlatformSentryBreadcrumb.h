#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryBreadcrumbAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryBreadcrumbApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryBreadcrumb.h"
#endif

static TSharedPtr<ISentryBreadcrumb> CreateSharedSentryBreadcrumb()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryBreadcrumbAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryBreadcrumbApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryBreadcrumb);
#else
	return nullptr;
#endif
}
