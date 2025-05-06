#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentrySpan.h"
#elif PLATFORM_APPLE
#include "Apple/SentrySpanApple.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentrySpan.h"
#else
#include "Null/NullSentrySpan.h"
#endif

static TSharedPtr<ISentrySpan> CreateSharedSentrySpan(const FString& Name, const FString& Operation)
{
	return MakeShareable(new FPlatformSentrySpan(Name, Operation));
}
