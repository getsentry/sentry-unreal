// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

static void PrintCrashLog(const sentry_ucontext_t* uctx)
{
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
	FWindowsSentryConverters::SentryCrashContextToString(uctx, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorExceptionDescription));

	const SIZE_T StackTraceSize = 65535;

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	ANSICHAR* StackTrace = (ANSICHAR*)FMemory::Malloc(StackTraceSize);
#else
	ANSICHAR* StackTrace = (ANSICHAR*)GMalloc->Malloc(StackTraceSize);
#endif // !UE_VERSION_OLDER_THAN(5, 6, 0)

	StackTrace[0] = 0;

	// Currently raw crash data stored in `uctx` can be utilized for stalk walking on Windows only
	void* ProgramCounter = uctx->exception_ptrs.ExceptionRecord->ExceptionAddress;

	FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, ProgramCounter);

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), GErrorExceptionDescription);
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), TEXT("\r\n\r\n"));
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), ANSI_TO_TCHAR(StackTrace));
#else
	FCString::Strncat(GErrorHist, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, TEXT("\r\n\r\n"), UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, ANSI_TO_TCHAR(StackTrace), UE_ARRAY_COUNT(GErrorHist));
#endif // !UE_VERSION_OLDER_THAN(5, 6, 0)

#if !NO_LOGGING
	FDebug::LogFormattedMessageWithCallstack(LogSentrySdk.GetCategoryName(), __FILE__, __LINE__, TEXT("=== Critical error: ==="), GErrorHist, ELogVerbosity::Error);
#endif // !NO_LOGGING

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	GLog->Panic();
#endif // !UE_VERSION_OLDER_THAN(5, 1, 0)

#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	FMemory::Free(StackTrace);
#else
	GMalloc->Free(StackTrace);
#endif // !UE_VERSION_OLDER_THAN(5, 6, 0)

#endif // !UE_VERSION_OLDER_THAN(5, 0, 0)
}

void FWindowsSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Crashpad executable couldn't be found so Breakpad will be used instead. Please make sure that the plugin was rebuilt to avoid initialization failure."));
		return;
	}

	sentry_options_set_handler_pathw(Options, *HandlerPath);
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	// Ensures that error message and corresponding callstack flushed to a log file (if available)
	// before it's attached to the captured crash event and uploaded to Sentry.
	PrintCrashLog(uctx);

	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
