// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryBreadcrumb.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryBreadcrumb.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryBreadcrumb.h"
#else
#include "Null/NullSentryBreadcrumb.h"
#endif

static TSharedPtr<ISentryBreadcrumb> CreateSharedSentryBreadcrumb()
{
	return MakeShareable(new FPlatformSentryBreadcrumb);
}
