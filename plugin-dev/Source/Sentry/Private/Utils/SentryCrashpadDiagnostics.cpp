// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryCrashpadDiagnostics.h"

#include "SentryDefines.h"
#include "Utils/SentryProtonUtils.h"

#if PLATFORM_WINDOWS
LPTOP_LEVEL_EXCEPTION_FILTER FSentryCrashpadDiagnostics::OriginalFilter = nullptr;
bool FSentryCrashpadDiagnostics::bTestFilterWasCalled = false;

LONG WINAPI FSentryCrashpadDiagnostics::TestExceptionFilter(EXCEPTION_POINTERS* ExceptionInfo)
{
	bTestFilterWasCalled = true;

	UE_LOG(LogSentrySdk, Error, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Error, TEXT("TEST EXCEPTION FILTER WAS CALLED!"));
	UE_LOG(LogSentrySdk, Error, TEXT("Exception Code: 0x%08X"), ExceptionInfo->ExceptionRecord->ExceptionCode);
	UE_LOG(LogSentrySdk, Error, TEXT("Exception Address: 0x%p"), ExceptionInfo->ExceptionRecord->ExceptionAddress);
	UE_LOG(LogSentrySdk, Error, TEXT("Wine's SetUnhandledExceptionFilter IS working!"));
	UE_LOG(LogSentrySdk, Error, TEXT("========================================"));

	// Call the original filter if it exists
	if (OriginalFilter)
	{
		return OriginalFilter(ExceptionInfo);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void FSentryCrashpadDiagnostics::InstallTestExceptionFilter()
{
#if PLATFORM_WINDOWS
	FSentryProtonUtils::FProtonInfo ProtonInfo = FSentryProtonUtils::DetectProtonEnvironment();

	if (!ProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Not running under Wine - skipping test exception filter"));
		return;
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Warning, TEXT("INSTALLING TEST EXCEPTION FILTER"));
	UE_LOG(LogSentrySdk, Warning, TEXT("This will test if Wine calls SetUnhandledExceptionFilter"));
	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));

	// Save the original filter
	OriginalFilter = SetUnhandledExceptionFilter(TestExceptionFilter);

	if (OriginalFilter)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Previous exception filter detected at: 0x%p"), OriginalFilter);
	}
	else
	{
		UE_LOG(LogSentrySdk, Log, TEXT("No previous exception filter was installed"));
	}
#endif
}

bool FSentryCrashpadDiagnostics::IsExceptionFilterInstalled()
{
#if PLATFORM_WINDOWS
	// Temporarily set a dummy filter to get the current one
	LPTOP_LEVEL_EXCEPTION_FILTER current = SetUnhandledExceptionFilter(nullptr);

	// Restore the current filter
	SetUnhandledExceptionFilter(current);

	bool isInstalled = (current != nullptr);

	UE_LOG(LogSentrySdk, Log, TEXT("Exception filter check: %s (0x%p)"),
		isInstalled ? TEXT("INSTALLED") : TEXT("NOT INSTALLED"),
		current);

	return isInstalled;
#else
	return false;
#endif
}

void FSentryCrashpadDiagnostics::TriggerTestCrash()
{
#if PLATFORM_WINDOWS
	UE_LOG(LogSentrySdk, Error, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Error, TEXT("TRIGGERING TEST CRASH"));
	UE_LOG(LogSentrySdk, Error, TEXT("This will intentionally crash the application!"));
	UE_LOG(LogSentrySdk, Error, TEXT("========================================"));

	// Trigger a null pointer dereference
	int* nullPtr = nullptr;
	*nullPtr = 42; // This will cause an access violation
#endif
}

void FSentryCrashpadDiagnostics::LogExceptionHandlerState()
{
#if PLATFORM_WINDOWS
	FSentryProtonUtils::FProtonInfo ProtonInfo = FSentryProtonUtils::DetectProtonEnvironment();

	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("CRASHPAD EXCEPTION HANDLER DIAGNOSTICS"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("Running under Wine/Proton: %s"), ProtonInfo.bIsRunningUnderWine ? TEXT("YES") : TEXT("NO"));

	if (ProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Wine Version: %s"), *ProtonInfo.WineVersion);
		UE_LOG(LogSentrySdk, Log, TEXT("Host System: %s"), *ProtonInfo.HostSystem);
	}

	bool filterInstalled = IsExceptionFilterInstalled();
	UE_LOG(LogSentrySdk, Log, TEXT("Exception Filter Installed: %s"), filterInstalled ? TEXT("YES") : TEXT("NO"));

	if (bTestFilterWasCalled)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Test Filter Was Called: YES"));
		UE_LOG(LogSentrySdk, Log, TEXT("  -> Wine IS calling SetUnhandledExceptionFilter"));
	}
	else
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Test Filter Was Called: NO (no crash triggered yet)"));
	}

	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("POTENTIAL ISSUES:"));

	if (ProtonInfo.bIsRunningUnderWine && !filterInstalled)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("  - No exception filter installed under Wine!"));
		UE_LOG(LogSentrySdk, Warning, TEXT("  - Crashpad may not have registered successfully"));
	}

	if (ProtonInfo.bIsRunningUnderWine && filterInstalled)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("  - Filter is installed, testing if Wine calls it..."));
		UE_LOG(LogSentrySdk, Log, TEXT("  - Possible issues:"));
		UE_LOG(LogSentrySdk, Log, TEXT("    1. Wine converts signals to SEH but doesn't call UnhandledExceptionFilter"));
		UE_LOG(LogSentrySdk, Log, TEXT("    2. Crashpad's IPC communication isn't working"));
		UE_LOG(LogSentrySdk, Log, TEXT("    3. Another module overrode Crashpad's filter"));
	}

	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("NEXT STEPS:"));
	UE_LOG(LogSentrySdk, Log, TEXT("  1. Check if crashpad_handler.exe is running (ps aux | grep crashpad)"));
	UE_LOG(LogSentrySdk, Log, TEXT("  2. Enable Wine debug channels: WINEDEBUG=+seh,+exception"));
	UE_LOG(LogSentrySdk, Log, TEXT("  3. Try triggering a test crash"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
#endif
}
