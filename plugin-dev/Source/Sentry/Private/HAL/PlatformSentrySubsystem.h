// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#ifdef SENTRY_PLATFORM_NULL
#include "Null/NullSentrySubsystem.h"
#else
#include COMPILED_PLATFORM_HEADER(SentrySubsystem.h)
#endif
