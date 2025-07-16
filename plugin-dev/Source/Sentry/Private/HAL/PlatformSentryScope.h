// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryScope.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryScope.h"
#elif USE_SENTRY_NATIVE && PLATFORM_MICROSOFT
#include "Microsoft/MicrosoftSentryScope.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryScope.h"
#else
#include "Null/NullSentryScope.h"
#endif
