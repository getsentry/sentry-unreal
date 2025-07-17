// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryModule.h"

// clang-format off

#if PLATFORM_MAC
	#define SENTRY_APPLE_CLASS(Name) \
		(__bridge Class) dlsym(FSentryModule::Get().GetSentryLibHandle(), "OBJC_CLASS_$_" #Name)
#elif PLATFORM_IOS
	#define SENTRY_APPLE_CLASS(Name) \
		Name
#endif

// clang-format on