// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#ifndef NS_SWIFT_SENDABLE
#define NS_SWIFT_SENDABLE
#endif

#if PLATFORM_MAC
#include <Sentry/Sentry.h>
#include <Sentry/PrivateSentrySDKOnly.h>
#include <Sentry/SentrySwift.h>
#elif PLATFORM_IOS
#import <Sentry/Sentry.h>
#import <Sentry/PrivateSentrySDKOnly.h>
#import <Sentry/SentrySwift.h>
#endif