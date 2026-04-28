// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "HAL/Platform.h"

#ifndef NS_SWIFT_SENDABLE
#define NS_SWIFT_SENDABLE
#endif

#if PLATFORM_MAC
#include <SentryObjC/SentryObjC.h>
#elif PLATFORM_IOS
#import <SentryObjC/SentryObjC.h>
#endif
#endif // !USE_SENTRY_NATIVE
