// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryWineVectoredExceptionHandler.h"

#include "SentryDefines.h"
#include "Utils/SentryProtonUtils.h"

#if PLATFORM_WINDOWS
PVOID FSentryWineVectoredExceptionHandler::VectoredHandlerHandle = nullptr;
LPTOP_LEVEL_EXCEPTION_FILTER FSentryWineVectoredExceptionHandler::CrashpadFilter = nullptr;
int32 FSentryWineVectoredExceptionHandler::ExceptionCount = 0;
bool FSentryWineVectoredExceptionHandler::bProcessingException = false;

LONG WINAPI FSentryWineVectoredExceptionHandler::VectoredExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	// Avoid recursion
	if (bProcessingException)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	ExceptionCount++;

	DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

	// Log that we caught the exception
	UE_LOG(LogSentrySdk, Error, TEXT("================================================"));
	UE_LOG(LogSentrySdk, Error, TEXT("VECTORED EXCEPTION HANDLER CAUGHT EXCEPTION!"));
	UE_LOG(LogSentrySdk, Error, TEXT("Exception #%d"), ExceptionCount);
	UE_LOG(LogSentrySdk, Error, TEXT("Code: 0x%08X"), exceptionCode);
	UE_LOG(LogSentrySdk, Error, TEXT("Address: 0x%p"), ExceptionInfo->ExceptionRecord->ExceptionAddress);

	// Filter out exceptions we don't want to handle
	// Let debugger breakpoints and single-steps through
	if (exceptionCode == EXCEPTION_BREAKPOINT ||
		exceptionCode == EXCEPTION_SINGLE_STEP ||
		exceptionCode == DBG_PRINTEXCEPTION_C ||
		exceptionCode == 0x406D1388) // MS VC++ SetThreadName exception
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Ignoring debug/system exception"));
		UE_LOG(LogSentrySdk, Error, TEXT("================================================"));
		return EXCEPTION_CONTINUE_SEARCH;
	}

	UE_LOG(LogSentrySdk, Error, TEXT("This is a CRASH exception - forwarding to Crashpad"));

	// Mark that we're processing to avoid recursion
	bProcessingException = true;

	// Try to call Crashpad's unhandled exception filter directly
	if (CrashpadFilter)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Calling Crashpad filter at: 0x%p"), CrashpadFilter);

		__try
		{
			LONG result = CrashpadFilter(ExceptionInfo);

			UE_LOG(LogSentrySdk, Error, TEXT("Crashpad filter returned: %d"), result);

			if (result == EXCEPTION_EXECUTE_HANDLER)
			{
				UE_LOG(LogSentrySdk, Error, TEXT("Crashpad handled the exception"));
			}
			else
			{
				UE_LOG(LogSentrySdk, Warning, TEXT("Crashpad passed on the exception"));
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			UE_LOG(LogSentrySdk, Error, TEXT("ERROR: Exception while calling Crashpad filter!"));
		}
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("ERROR: No Crashpad filter available!"));
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad may not have initialized properly"));
	}

	UE_LOG(LogSentrySdk, Error, TEXT("================================================"));

	bProcessingException = false;

	// Continue searching so other handlers (like Wine's crash dialog) can run
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void FSentryWineVectoredExceptionHandler::Install()
{
#if PLATFORM_WINDOWS
	FSentryProtonUtils::FProtonInfo ProtonInfo = FSentryProtonUtils::DetectProtonEnvironment();

	if (!ProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Not running under Wine - skipping Vectored Exception Handler"));
		return;
	}

	if (VectoredHandlerHandle != nullptr)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Vectored Exception Handler already installed"));
		return;
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Warning, TEXT("INSTALLING VECTORED EXCEPTION HANDLER FOR WINE/PROTON"));
	UE_LOG(LogSentrySdk, Warning, TEXT("Wine doesn't call SetUnhandledExceptionFilter reliably"));
	UE_LOG(LogSentrySdk, Warning, TEXT("But it DOES support AddVectoredExceptionHandler!"));
	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));

	// Get Crashpad's current unhandled exception filter
	// We'll call this manually when we catch exceptions
	CrashpadFilter = SetUnhandledExceptionFilter(nullptr);
	SetUnhandledExceptionFilter(CrashpadFilter); // Restore it

	if (CrashpadFilter)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Found Crashpad's filter at: 0x%p"), CrashpadFilter);
	}
	else
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("WARNING: No unhandled exception filter found!"));
		UE_LOG(LogSentrySdk, Warning, TEXT("Crashpad may not have initialized yet"));
	}

	// Install our vectored exception handler
	// First = 1 means we want to be called FIRST, before other VEH handlers
	VectoredHandlerHandle = AddVectoredExceptionHandler(1, VectoredExceptionHandler);

	if (VectoredHandlerHandle)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Successfully installed Vectored Exception Handler"));
		UE_LOG(LogSentrySdk, Log, TEXT("Handler address: 0x%p"), VectoredExceptionHandler);
		UE_LOG(LogSentrySdk, Log, TEXT(""));
		UE_LOG(LogSentrySdk, Log, TEXT("This handler will:"));
		UE_LOG(LogSentrySdk, Log, TEXT("  1. Catch ALL exceptions before other handlers"));
		UE_LOG(LogSentrySdk, Log, TEXT("  2. Forward crash exceptions to Crashpad manually"));
		UE_LOG(LogSentrySdk, Log, TEXT("  3. Work around Wine's SetUnhandledExceptionFilter issue"));
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("ERROR: Failed to install Vectored Exception Handler!"));
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));
#endif
}

void FSentryWineVectoredExceptionHandler::Uninstall()
{
#if PLATFORM_WINDOWS
	if (VectoredHandlerHandle)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Uninstalling Vectored Exception Handler"));
		RemoveVectoredExceptionHandler(VectoredHandlerHandle);
		VectoredHandlerHandle = nullptr;
		CrashpadFilter = nullptr;
	}
#endif
}

bool FSentryWineVectoredExceptionHandler::IsInstalled()
{
#if PLATFORM_WINDOWS
	return VectoredHandlerHandle != nullptr;
#else
	return false;
#endif
}
