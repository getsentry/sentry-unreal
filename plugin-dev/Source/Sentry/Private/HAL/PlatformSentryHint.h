// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryHint.h"
#elif PLATFORM_APPLE && !USE_SENTRY_NATIVE
#include "Apple/AppleSentryHint.h"
#else
#include "Null/NullSentryHint.h"
#endif

static TSharedPtr<ISentryHint> CreateSharedSentryHint()
{
	return MakeShareable(new FPlatformSentryHint());
}
