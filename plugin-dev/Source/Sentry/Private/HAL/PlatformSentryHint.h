// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryHint.h"
#elif PLATFORM_APPLE && !USE_SENTRY_NATIVE
#include "Apple/AppleSentryHint.h"
#elif USE_SENTRY_NATIVE && PLATFORM_MICROSOFT
#include "Microsoft/MicrosoftSentryHint.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryHint.h"
#else
#include "Null/NullSentryHint.h"
#endif

static TSharedPtr<ISentryHint> CreateSharedSentryHint()
{
	return MakeShareable(new FPlatformSentryHint());
}
