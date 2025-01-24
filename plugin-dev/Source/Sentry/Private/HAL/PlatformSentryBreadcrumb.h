#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryBreadcrumbAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryBreadcrumbApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryBreadcrumb.h"
#endif

static TSharedPtr<ISentryBreadcrumb> CreateSharedSentryBreadcrumb()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryBreadcrumbAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryBreadcrumbApple);
#else
	return MakeShareable(new FGenericPlatformSentryBreadcrumb);
#endif
}
