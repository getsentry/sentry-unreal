// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <windows.h>
#endif

/**
 * Vectored Exception Handler for Wine/Proton compatibility.
 *
 * Wine doesn't reliably call SetUnhandledExceptionFilter handlers,
 * but it DOES support AddVectoredExceptionHandler which is called
 * BEFORE frame-based handlers and SetUnhandledExceptionFilter.
 *
 * This handler catches exceptions under Wine/Proton and forwards them
 * to Crashpad's exception handler manually.
 */
class FSentryWineVectoredExceptionHandler
{
public:
	/**
	 * Installs a Vectored Exception Handler for Wine/Proton.
	 * This should be called AFTER Sentry/Crashpad initialization.
	 */
	static void Install();

	/**
	 * Removes the Vectored Exception Handler.
	 */
	static void Uninstall();

	/**
	 * Checks if the handler is currently installed.
	 */
	static bool IsInstalled();

private:
#if PLATFORM_WINDOWS
	/** The vectored exception handler callback */
	static LONG WINAPI VectoredExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo);

	/** Handle to the installed vectored exception handler */
	static PVOID VectoredHandlerHandle;

	/** Crashpad's original unhandled exception filter */
	static LPTOP_LEVEL_EXCEPTION_FILTER CrashpadFilter;

	/** Count of exceptions handled */
	static int32 ExceptionCount;

	/** Whether we're currently processing an exception (to avoid recursion) */
	static bool bProcessingException;
#endif
};
