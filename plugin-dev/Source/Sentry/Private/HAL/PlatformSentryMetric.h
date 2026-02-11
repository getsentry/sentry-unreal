// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryMetric.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryMetric.h"
#else
#include "Null/NullSentryMetric.h"
#endif

static TSharedPtr<ISentryMetric> CreateSharedSentryMetric()
{
	return MakeShareable(new FPlatformSentryMetric);
}
