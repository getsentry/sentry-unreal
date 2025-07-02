// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "CoreMinimal.h"

#include "GenericPlatform/Convenience/GenericPlatformSentryInclude.h"

class FWindowsSentryConverters
{
public:
	static void SentryCrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize);
};

#endif // USE_SENTRY_NATIVE
