#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryUserFeedbackAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryUserFeedbackApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryUserFeedback.h"
#endif

static TSharedPtr<ISentryUserFeedback> CreateSharedSentryUserFeedback(TSharedPtr<ISentryId> EventId)
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryUserFeedbackAndroid(EventId));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryUserFeedbackApple(EventId));
#else
	return MakeShareable(new FGenericPlatformSentryUserFeedback(EventId));
#endif
}
