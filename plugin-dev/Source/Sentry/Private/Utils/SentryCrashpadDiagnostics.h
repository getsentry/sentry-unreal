// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <windows.h>
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

	/**
	 * Installs a wrapper exception filter that logs when Wine calls it,
	 * then forwards to the original (Crashpad's) filter.
	 * This helps diagnose if Wine is calling the filter but Crashpad's IPC is failing.
	 */
	static void InstallLoggingExceptionFilterWrapper();

private:
#if PLATFORM_WINDOWS
	/** Test exception filter callback */
	static LONG WINAPI TestExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo);

	/** Logging wrapper exception filter that monitors calls */
	static LONG WINAPI LoggingExceptionFilterWrapper(EXCEPTION_POINTERS* ExceptionInfo);

	/** Original exception filter before our test */
	static LPTOP_LEVEL_EXCEPTION_FILTER OriginalFilter;

	/** Crashpad's exception filter that we're wrapping */
	static LPTOP_LEVEL_EXCEPTION_FILTER CrashpadFilter;

	/** Whether our test filter was called */
	static bool bTestFilterWasCalled;

	/** Whether the logging wrapper was called */
	static bool bLoggingWrapperCalled;

	/** Number of times the wrapper was called */
	static int32 WrapperCallCount;
#endif
};
