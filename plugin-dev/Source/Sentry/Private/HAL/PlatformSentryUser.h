#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryUserAndroid.h"
#define NEW_SENTRY_USER new SentryUserAndroid()
#elif PLATFORM_APPLE
#include "Apple/SentryUserApple.h"
#define NEW_SENTRY_USER new SentryUserApple()
#else
#include "GenericPlatform/GenericPlatformSentryUser.h"
#define NEW_SENTRY_USER new FGenericPlatformSentryUser()
#endif

#ifndef NEW_SENTRY_USER
#error Make sure the NEW_SENTRY_USER macro is defined for supported platforms
#endif
