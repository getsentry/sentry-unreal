// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryId.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryId.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryId.h"
#else
#include "Null/NullSentryId.h"
#endif

static TSharedPtr<ISentryId> CreateSharedSentryId(const FString& Id)
{
	return MakeShareable(new FPlatformSentryId(Id));
}
