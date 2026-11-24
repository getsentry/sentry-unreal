// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryEvent.h"
#elif PLATFORM_APPLE && !USE_SENTRY_NATIVE
#include "Apple/AppleSentryEvent.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryEvent.h"
#else
#include "Null/NullSentryEvent.h"
#endif

static TSharedPtr<ISentryEvent> CreateSharedSentryEvent()
{
	return MakeShareable(new FPlatformSentryEvent);
}
