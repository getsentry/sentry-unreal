#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryBreadcrumbAndroid.h"
#define NEW_SENTRY_BREADCRUMB new SentryBreadcrumbAndroid()
#elif PLATFORM_APPLE
#include "Apple/SentryBreadcrumbApple.h"
#define NEW_SENTRY_BREADCRUMB new SentryBreadcrumbApple()
#else
#include "GenericPlatform/GenericPlatformSentryBreadcrumb.h"
#define NEW_SENTRY_BREADCRUMB new FGenericPlatformSentryBreadcrumb()
#endif

#ifndef NEW_SENTRY_BREADCRUMB
#error Make sure the NEW_SENTRY_BREADCRUMB macro is defined for supported platforms
#endif
