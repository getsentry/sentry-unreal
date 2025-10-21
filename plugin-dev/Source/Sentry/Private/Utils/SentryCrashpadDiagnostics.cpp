// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryCrashpadDiagnostics.h"

#include "SentryDefines.h"
#include "Utils/SentryProtonUtils.h"

#if PLATFORM_WINDOWS
LPTOP_LEVEL_EXCEPTION_FILTER FSentryCrashpadDiagnostics::OriginalFilter = nullptr;
LPTOP_LEVEL_EXCEPTION_FILTER FSentryCrashpadDiagnostics::CrashpadFilter = nullptr;
bool FSentryCrashpadDiagnostics::bTestFilterWasCalled = false;
bool FSentryCrashpadDiagnostics::bLoggingWrapperCalled = false;
int32 FSentryCrashpadDiagnostics::WrapperCallCount = 0;

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

LONG WINAPI FSentryCrashpadDiagnostics::LoggingExceptionFilterWrapper(EXCEPTION_POINTERS* ExceptionInfo)
{
	bLoggingWrapperCalled = true;
	WrapperCallCount++;

	UE_LOG(LogSentrySdk, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	UE_LOG(LogSentrySdk, Error, TEXT("LOGGING WRAPPER CALLED! (Call #%d)"), WrapperCallCount);
	UE_LOG(LogSentrySdk, Error, TEXT("Wine IS calling the exception filter!"));
	UE_LOG(LogSentrySdk, Error, TEXT("Exception Code: 0x%08X"), ExceptionInfo->ExceptionRecord->ExceptionCode);
	UE_LOG(LogSentrySdk, Error, TEXT("Exception Address: 0x%p"), ExceptionInfo->ExceptionRecord->ExceptionAddress);
	UE_LOG(LogSentrySdk, Error, TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));

	// Forward to Crashpad's original filter
	if (CrashpadFilter)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Calling Crashpad's filter at 0x%p..."), CrashpadFilter);
		LONG result = CrashpadFilter(ExceptionInfo);
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad filter returned: %d"), result);

		if (result == EXCEPTION_EXECUTE_HANDLER)
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Crashpad wants to handle the exception"));
		}
		else if (result == EXCEPTION_CONTINUE_SEARCH)
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Crashpad is passing on the exception"));
		}

		return result;
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("ERROR: No Crashpad filter to forward to!"));
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

	// Try to identify which module owns the filter
	if (current)
	{
		HMODULE hModule = nullptr;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCTSTR)current, &hModule))
		{
			WCHAR modulePath[MAX_PATH];
			if (GetModuleFileNameW(hModule, modulePath, MAX_PATH))
			{
				UE_LOG(LogSentrySdk, Log, TEXT("  Filter belongs to module: %s"), modulePath);

				FString modulePathStr(modulePath);
				if (modulePathStr.Contains(TEXT("crashpad"), ESearchCase::IgnoreCase))
				{
					UE_LOG(LogSentrySdk, Log, TEXT("  -> Appears to be Crashpad's handler"));
				}
				else if (modulePathStr.Contains(TEXT("steam"), ESearchCase::IgnoreCase))
				{
					UE_LOG(LogSentrySdk, Warning, TEXT("  -> WARNING: Appears to be Steam's handler!"));
					UE_LOG(LogSentrySdk, Warning, TEXT("  -> Steam overlay may interfere with Crashpad"));
				}
				else if (modulePathStr.Contains(TEXT("sentry"), ESearchCase::IgnoreCase))
				{
					UE_LOG(LogSentrySdk, Log, TEXT("  -> Appears to be Sentry's handler"));
				}
			}
		}
	}

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
		UE_LOG(LogSentrySdk, Log, TEXT("Host System: %s %s"), *ProtonInfo.HostSystem, *ProtonInfo.HostRelease);

		if (ProtonInfo.bIsSteamProton)
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Steam Proton: YES"));
			UE_LOG(LogSentrySdk, Log, TEXT("Proton Version: %s"), *ProtonInfo.ProtonVersion);
			UE_LOG(LogSentrySdk, Log, TEXT(""));
			UE_LOG(LogSentrySdk, Warning, TEXT("STEAM OVERLAY WARNING:"));
			UE_LOG(LogSentrySdk, Warning, TEXT("Steam runs its own native Linux crashpad_handler"));
			UE_LOG(LogSentrySdk, Warning, TEXT("This may conflict with your Windows Crashpad via Proton!"));
			UE_LOG(LogSentrySdk, Warning, TEXT(""));
		}
	}

	UE_LOG(LogSentrySdk, Log, TEXT(""));
	bool filterInstalled = IsExceptionFilterInstalled();

	UE_LOG(LogSentrySdk, Log, TEXT(""));
	if (bTestFilterWasCalled)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Test Filter Called: YES"));
		UE_LOG(LogSentrySdk, Log, TEXT("  -> Wine IS calling SetUnhandledExceptionFilter"));
		UE_LOG(LogSentrySdk, Log, TEXT("  -> Problem is likely IPC or handler communication"));
	}
	else
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Test Filter Called: NO (trigger crash to test)"));
	}

	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("WHERE TO FIND WINE DEBUG LOGS:"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("1. If running from terminal with WINEDEBUG:"));
	UE_LOG(LogSentrySdk, Log, TEXT("   -> Output goes to stderr"));
	UE_LOG(LogSentrySdk, Log, TEXT("   -> Redirect: WINEDEBUG=+seh,+exception %%command%% 2>&1 | tee ~/wine.log"));
	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("2. Steam launch options (add this):"));
	UE_LOG(LogSentrySdk, Log, TEXT("   WINEDEBUG=+seh,+exception,+relay,+pipe %%command%% 2>&1 | tee ~/wine_debug.log"));
	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("3. Check standard Steam/Proton log locations:"));
	UE_LOG(LogSentrySdk, Log, TEXT("   ~/.steam/steam/logs/"));
	UE_LOG(LogSentrySdk, Log, TEXT("   ~/.steam/steam/steamapps/compatdata/<AppID>/"));
	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("DEBUGGING STEPS:"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("1. Check running crashpad processes:"));
	UE_LOG(LogSentrySdk, Log, TEXT("   ps aux | grep crashpad"));
	UE_LOG(LogSentrySdk, Log, TEXT("   -> You should see both Steam's and your game's handler"));
	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("2. Enable Wine debugging and trigger crash"));
	UE_LOG(LogSentrySdk, Log, TEXT("3. Check Wine logs for:"));
	UE_LOG(LogSentrySdk, Log, TEXT("   - SEH exception being raised"));
	UE_LOG(LogSentrySdk, Log, TEXT("   - UnhandledExceptionFilter being called"));
	UE_LOG(LogSentrySdk, Log, TEXT("   - Named pipe communication"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));

	UE_LOG(LogSentrySdk, Log, TEXT(""));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Log, TEXT("POTENTIAL ISSUES:"));
	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));

	if (ProtonInfo.bIsRunningUnderWine && !filterInstalled)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("  [CRITICAL] No exception filter installed!"));
		UE_LOG(LogSentrySdk, Error, TEXT("  -> Crashpad StartHandler() failed"));
	}
	else if (ProtonInfo.bIsRunningUnderWine && filterInstalled)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("  [INFO] Filter IS installed but crashes may not be captured"));
		UE_LOG(LogSentrySdk, Warning, TEXT("  Possible causes:"));
		UE_LOG(LogSentrySdk, Warning, TEXT("    1. Wine doesn't call filter for all exception types"));
		UE_LOG(LogSentrySdk, Warning, TEXT("    2. Named pipe IPC fails between process and handler"));
		UE_LOG(LogSentrySdk, Warning, TEXT("    3. Steam overlay's handler interferes"));
		UE_LOG(LogSentrySdk, Warning, TEXT("    4. Another module overrides the filter"));
	}

	if (bLoggingWrapperCalled)
	{
		UE_LOG(LogSentrySdk, Error, TEXT(""));
		UE_LOG(LogSentrySdk, Error, TEXT("!!! LOGGING WRAPPER WAS CALLED %d TIMES !!!"), WrapperCallCount);
		UE_LOG(LogSentrySdk, Error, TEXT("This means Wine IS calling the exception filter!"));
		UE_LOG(LogSentrySdk, Error, TEXT("The problem is either:"));
		UE_LOG(LogSentrySdk, Error, TEXT("  1. Crashpad's filter is failing internally"));
		UE_LOG(LogSentrySdk, Error, TEXT("  2. IPC with crashpad_handler.exe is broken"));
		UE_LOG(LogSentrySdk, Error, TEXT("  3. Crash report is generated but not uploaded"));
	}

	UE_LOG(LogSentrySdk, Log, TEXT("========================================"));
#endif
}

void FSentryCrashpadDiagnostics::InstallLoggingExceptionFilterWrapper()
{
#if PLATFORM_WINDOWS
	FSentryProtonUtils::FProtonInfo ProtonInfo = FSentryProtonUtils::DetectProtonEnvironment();

	if (!ProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Not running under Wine - skipping logging wrapper"));
		return;
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));
	UE_LOG(LogSentrySdk, Warning, TEXT("INSTALLING LOGGING EXCEPTION FILTER WRAPPER"));
	UE_LOG(LogSentrySdk, Warning, TEXT("This will intercept exception filter calls and log them"));
	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));

	// Get Crashpad's current filter
	CrashpadFilter = SetUnhandledExceptionFilter(LoggingExceptionFilterWrapper);

	if (CrashpadFilter)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Wrapped Crashpad's filter at: 0x%p"), CrashpadFilter);
		UE_LOG(LogSentrySdk, Log, TEXT("Our wrapper is now at: 0x%p"), LoggingExceptionFilterWrapper);
		UE_LOG(LogSentrySdk, Log, TEXT("When a crash occurs, you'll see detailed logs about:"));
		UE_LOG(LogSentrySdk, Log, TEXT("  1. Whether Wine calls the filter"));
		UE_LOG(LogSentrySdk, Log, TEXT("  2. What Crashpad's filter returns"));
		UE_LOG(LogSentrySdk, Log, TEXT("  3. Whether the problem is in Crashpad or Wine"));
	}
	else
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("No previous filter was installed - Crashpad may not have initialized"));
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("========================================"));
#endif
}
