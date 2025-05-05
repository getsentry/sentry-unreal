#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryEvent.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryEvent.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryEvent.h"
#else
#include "Interface/SentryEventInterface.h"
#endif

static TSharedPtr<ISentryEvent> CreateSharedSentryEvent()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryEvent);
#elif PLATFORM_APPLE
	return MakeShareable(new FAppleSentryEvent);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryEvent);
#else
	return nullptr;
#endif
}
