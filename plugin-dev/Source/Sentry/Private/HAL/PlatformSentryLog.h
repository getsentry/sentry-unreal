// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryLog.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryLog.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryLog.h"
#else
#include "Null/NullSentryLog.h"
#endif

static TSharedPtr<ISentryLog> CreateSharedSentryLog()
{
	return MakeShareable(new FPlatformSentryLog);
}