// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

#include "SentryModule.h"

#include <objc/objc.h>

#if PLATFORM_MAC
	#define SENTRY_APPLE_CLASS(Name) \
		(__bridge Class)dlsym(FSentryModule::Get().GetSentryLibHandle(), "OBJC_CLASS_$_" #Name)
#elif PLATFORM_IOS
	#define SENTRY_APPLE_CLASS(Name) \
		Name
#endif