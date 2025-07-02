// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryUserFeedback.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryUserFeedback.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryUserFeedback.h"
#else
#include "Null/NullSentryUserFeedback.h"
#endif

#include "PlatformSentryId.h"

static TSharedPtr<ISentryUserFeedback> CreateSharedSentryUserFeedback(const FString& EventId)
{
	TSharedPtr<ISentryId> Id = MakeShareable(new FPlatformSentryId(EventId));

	return MakeShareable(new FPlatformSentryUserFeedback(Id));
}
