#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryUserFeedback.h"
#elif PLATFORM_APPLE
#include "Apple/SentryUserFeedbackApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryUserFeedback.h"
#else
#include "Interface/SentryUserFeedbackInterface.h"
#endif

#include "PlatformSentryId.h"

static TSharedPtr<ISentryUserFeedback> CreateSharedSentryUserFeedback(const FString& EventId)
{
	TSharedPtr<ISentryId> Id = CreateSharedSentryId(EventId);

#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryUserFeedback(Id));
#elif PLATFORM_APPLE
	return MakeShareable(new SentryUserFeedbackApple(Id));
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryUserFeedback(Id));
#else
	return nullptr;
#endif
}
