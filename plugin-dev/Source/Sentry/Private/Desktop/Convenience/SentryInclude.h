// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "sentry.h"
#include "Windows/HideWindowsPlatformTypes.h"
#elif PLATFORM_MAC || PLATFORM_LINUX
#include "sentry.h"
#endif