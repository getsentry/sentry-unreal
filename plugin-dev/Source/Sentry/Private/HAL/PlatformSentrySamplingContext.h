// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_ANDROID
#include "Android/AndroidSentrySamplingContext.h"
#elif PLATFORM_APPLE
#include "Apple/AppleSentrySamplingContext.h"
#else
#include "Null/NullSentrySamplingContext.h"
#endif
