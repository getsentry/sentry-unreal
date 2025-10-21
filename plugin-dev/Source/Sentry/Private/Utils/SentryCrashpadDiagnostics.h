// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Diagnostics for Crashpad exception handling on Wine/Proton.
 *
 * This utility helps diagnose why Crashpad's crash handler isn't catching
 * crashes under Wine/Proton even when crashpad_handler.exe is running.
 */
class FSentryCrashpadDiagnostics
{
public:
	/**
	 * Installs a test exception filter to verify Wine is calling SetUnhandledExceptionFilter.
	 * This should be called BEFORE Sentry initialization to test Wine's SEH implementation.
	 */
	static void InstallTestExceptionFilter();

	/**
	 * Verifies if an unhandled exception filter is currently installed.
	 * Returns true if a filter is set (non-null).
	 */
	static bool IsExceptionFilterInstalled();

	/**
	 * Attempts to manually trigger a crash to test exception handling.
	 * WARNING: This WILL crash your application! Only use for testing.
	 */
	static void TriggerTestCrash();

	/**
	 * Logs diagnostic information about the current exception handling state.
	 */
	static void LogExceptionHandlerState();

private:
#if PLATFORM_WINDOWS
	/** Test exception filter callback */
	static LONG WINAPI TestExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo);

	/** Original exception filter before our test */
	static LPTOP_LEVEL_EXCEPTION_FILTER OriginalFilter;

	/** Whether our test filter was called */
	static bool bTestFilterWasCalled;
#endif
};
