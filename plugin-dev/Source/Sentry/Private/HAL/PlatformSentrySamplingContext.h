// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_ANDROID
#include "Android/AndroidSentrySamplingContext.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentrySamplingContext.h"
#elif USE_SENTRY_NATIVE
#include "GenericPlatform/GenericPlatformSentrySamplingContext.h"
#else
#include "Null/NullSentrySamplingContext.h"
#endif
