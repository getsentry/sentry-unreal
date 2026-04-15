// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "HAL/Platform.h"

#ifndef NS_SWIFT_SENDABLE
#define NS_SWIFT_SENDABLE
#endif

#if PLATFORM_MAC
#include <Sentry/PrivateSentrySDKOnly.h>
#include <Sentry/Sentry.h>
#include <Sentry/SentrySwift.h>
#elif PLATFORM_IOS
#import <Sentry/PrivateSentrySDKOnly.h>
#import <Sentry/Sentry.h>
#import <Sentry/SentrySwift.h>
#endif

#endif // !USE_SENTRY_NATIVE