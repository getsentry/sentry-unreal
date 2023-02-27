// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#if PLATFORM_MAC
#include "Headers/Sentry.h"
#include "Headers/PrivateSentrySDKOnly.h"
#elif PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>
#import <Sentry/PrivateSentrySDKOnly.h>
#endif