#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryBreadcrumb.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryBreadcrumb.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryBreadcrumb.h"
#else
#include "Interface/SentryBreadcrumbInterface.h"
#endif

static TSharedPtr<ISentryBreadcrumb> CreateSharedSentryBreadcrumb()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryBreadcrumb);
#elif PLATFORM_APPLE
	return MakeShareable(new FAppleSentryBreadcrumb);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryBreadcrumb);
#else
	return nullptr;
#endif
}
