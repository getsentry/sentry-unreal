#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryUserFeedbackAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryUserFeedbackApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryUserFeedback.h"
#endif

static TSharedPtr<ISentryUserFeedback> CreateSharedSentryUserFeedback(TSharedPtr<ISentryId> EventId)
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryUserFeedbackAndroid(EventId));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryUserFeedbackApple(EventId));
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryUserFeedback(EventId));
#else
	return nullptr;
#endif
}
