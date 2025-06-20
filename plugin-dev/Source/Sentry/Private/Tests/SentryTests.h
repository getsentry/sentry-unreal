// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"

#if WITH_AUTOMATION_TESTS

#if UE_VERSION_OLDER_THAN(5, 5, 0)
static constexpr EAutomationTestFlags::Type SentryApplicationContextMask = EAutomationTestFlags::ApplicationContextMask;
#else
static constexpr EAutomationTestFlags SentryApplicationContextMask = EAutomationTestFlags_ApplicationContextMask;
#endif

#endif