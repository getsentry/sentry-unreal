// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#ifdef SENTRY_PLATFORM_NULL
#include "Null/NullSentryScope.h"
#else
#if PLATFORM_ANDROID
#include "Android/AndroidSentryScope.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryScope.h"
#elif PLATFORM_MICROSOFT
#include "Microsoft/MicrosoftSentryScope.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryScope.h"
#else
#include "Null/NullSentryScope.h"
#endif
#endif
