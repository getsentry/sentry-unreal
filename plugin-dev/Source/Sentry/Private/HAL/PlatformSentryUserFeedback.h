#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryUserFeedbackAndroid.h"
#define NEW_USER_FEEDBACK(EventId) new SentryUserFeedbackAndroid(EventId)
#elif PLATFORM_APPLE
#include "Apple/SentryUserFeedbackApple.h"
#define NEW_USER_FEEDBACK(EventId) new SentryUserFeedbackApple(EventId)
#else
#include "GenericPlatform/GenericPlatformSentryUserFeedback.h"
#define NEW_USER_FEEDBACK(EventId) new FGenericPlatformSentryUserFeedback(EventId)
#endif

#ifndef NEW_USER_FEEDBACK
#error Make sure the NEW_USER_FEEDBACK macro is defined for supported platforms
#endif
