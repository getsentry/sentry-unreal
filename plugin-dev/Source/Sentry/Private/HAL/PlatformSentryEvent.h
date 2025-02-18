#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryEventAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryEventApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryEvent.h"
#endif

static TSharedPtr<ISentryEvent> CreateSharedSentryEvent()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryEventAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryEventApple);
#else
	return MakeShareable(new FGenericPlatformSentryEvent);
#endif
}
