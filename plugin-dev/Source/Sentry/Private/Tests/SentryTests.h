// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 4, 0)
static constexpr EAutomationTestFlags::Type SentryApplicationContextMask = EAutomationTestFlags::ApplicationContextMask;
#else
static constexpr EAutomationTestFlags SentryApplicationContextMask = EAutomationTestFlags_ApplicationContextMask;
#endif