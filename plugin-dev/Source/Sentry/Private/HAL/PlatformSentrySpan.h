#pragma once

#if PLATFORM_ANDROID
#include "Android/SentrySpanAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentrySpanApple.h"
#else
#include "GenericPlatform/GenericPlatformSentrySpan.h"
#endif

static TSharedPtr<ISentrySpan> CreateSharedSentrySpan(const FString& Name, const FString& Operation)
{
#if PLATFORM_ANDROID
	return MakeShareable(new SentrySpanAndroid);
#elif PLATFORM_APPLE
	return MakeShareable(new SentrySpanApple);
#else
	return MakeShareable(new FGenericPlatformSentrySpan);
#endif
}
