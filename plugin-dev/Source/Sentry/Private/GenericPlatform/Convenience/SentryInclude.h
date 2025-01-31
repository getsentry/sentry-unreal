// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_WINDOWS
#include "Microsoft/WindowsHWrapper.h"
#endif // PLATFORM_WINDOWS

#if USE_SENTRY_NATIVE
#include "sentry.h"
#endif // USE_SENTRY_NATIVE
