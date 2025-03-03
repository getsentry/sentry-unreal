#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryEventAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryEventApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryEvent.h"
#else
#include "Interface/SentryEventInterface.h"
#endif

static TSharedPtr<ISentryEvent> CreateSharedSentryEvent()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryEventAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryEventApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryEvent);
#else
	return nullptr;
#endif
}
