// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentrySpan.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentrySpan.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentrySpan.h"
#else
#include "Null/NullSentrySpan.h"
#endif
