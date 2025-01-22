#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryEventAndroid.h"
#define NEW_SENTRY_EVENT new SentryEventAndroid()
#elif PLATFORM_APPLE
#include "Apple/SentryEventApple.h"
#define NEW_SENTRY_EVENT new SentryEventApple()
#else
#include "GenericPlatform/GenericPlatformSentryEvent.h"
#define NEW_SENTRY_EVENT new FGenericPlatformSentryEvent()
#endif

#ifndef NEW_SENTRY_EVENT
#error Make sure the NEW_SENTRY_EVENT macro is defined for supported platforms
#endif
