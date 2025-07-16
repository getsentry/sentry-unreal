// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentryUser.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentryUser.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentryUser.h"
#else
#include "Null/NullSentryUser.h"
#endif