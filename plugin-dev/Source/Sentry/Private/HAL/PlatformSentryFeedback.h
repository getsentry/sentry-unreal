// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentryFeedback.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryFeedback.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryFeedback.h"
#else
#include "Null/NullSentryFeedback.h"
#endif
