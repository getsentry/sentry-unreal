// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_MICROSOFT
#include "Microsoft/WindowsHWrapper.h"
#endif // PLATFORM_MICROSOFT

#if USE_SENTRY_NATIVE
#include "sentry.h"
#endif // USE_SENTRY_NATIVE
