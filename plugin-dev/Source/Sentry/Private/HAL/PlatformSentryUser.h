#pragma once

#if PLATFORM_ANDROID
#include "Android/SentryUserAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentryUserApple.h"
#else
#include "GenericPlatform/GenericPlatformSentryUser.h"
#endif

static TSharedPtr<ISentryUser> CreateSharedSentryUser()
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentryUserAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentryUserApple);
#else
	return MakeShareable(new FGenericPlatformSentryUser);
#endif
}
