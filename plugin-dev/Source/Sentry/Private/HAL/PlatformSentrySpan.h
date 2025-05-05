#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentrySpan.h"
#elif PLATFORM_APPLE
#include "Apple/SentrySpanApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentrySpan.h"
#else
#include "Interface/SentrySpanInterface.h"
#endif

static TSharedPtr<ISentrySpan> CreateSharedSentrySpan(const FString& Name, const FString& Operation)
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentrySpan);
#elif PLATFORM_APPLE
	return MakeShareable(new SentrySpanApple);
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentrySpan);
#else
	return nullptr;
#endif
}
