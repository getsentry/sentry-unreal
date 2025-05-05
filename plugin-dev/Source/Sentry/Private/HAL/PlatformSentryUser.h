#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryUser.h"
#elif PLATFORM_APPLE
#include "Apple/SentryUserApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryUser.h"
#else
#include "Interface/SentryUserInterface.h"
#endif

static TSharedPtr<ISentryUser> CreateSharedSentryUser()
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryUser);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryUserApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryUser);
#else
	return nullptr;
#endif
}
