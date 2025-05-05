#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryId.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryId.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryId.h"
#else
#include "Interface/SentryIdInterface.h"
#endif

static TSharedPtr<ISentryId> CreateSharedSentryId(const FString& Id)
{
#if PLATFORM_ANDROID
	return MakeShareable(new FAndroidSentryId(Id));
#elif PLATFORM_APPLE
	return MakeShareable(new FAppleSentryId(Id));
#elif USE_SENTRY_NATIVE
	return MakeShareable(new FGenericPlatformSentryId(Id));
#else
	return nullptr;
#endif
}
