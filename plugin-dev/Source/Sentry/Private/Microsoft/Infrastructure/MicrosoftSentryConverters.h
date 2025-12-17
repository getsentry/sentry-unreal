// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "CoreMinimal.h"

#include "GenericPlatform/Convenience/GenericPlatformSentryInclude.h"

/**
 * Utility class for converting Sentry crash context to human-readable strings.
 * Shared across all Microsoft platforms (Windows, Xbox).
 */
class FMicrosoftSentryConverters
{
public:
	/**
	 * Converts Sentry crash context to a human-readable exception string.
	 *
	 * @param crashContext - Sentry crash context containing exception information
	 * @param outErrorString - Output buffer for the error string
	 * @param errorStringBufSize - Size of the output buffer
	 */
	static void SentryCrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize);
};

#endif // USE_SENTRY_NATIVE
