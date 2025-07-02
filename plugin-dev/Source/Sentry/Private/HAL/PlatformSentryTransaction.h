// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryTransaction.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryTransaction.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryTransaction.h"
#else
#include "Null/NullSentryTransaction.h"
#endif
