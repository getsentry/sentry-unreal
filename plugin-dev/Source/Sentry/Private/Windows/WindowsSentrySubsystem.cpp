#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Windows/WindowsPlatformStackWalk.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"

void FWindowsSentrySubsystem::ConfigureGpuDumpAttachment(sentry_options_t *Options)
{
	sentry_options_add_attachmentw(Options, *GetGpuDumpBackupPath());
}

static void PrintCrashLog(const sentry_ucontext_t *uctx)
{
#if !UE_VERSION_OLDER_THAN(5, 0, 0)
	FWindowsSentryConverters::SentryCrashContextToString(uctx, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorExceptionDescription));

	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)GMalloc->Malloc(StackTraceSize);
	StackTrace[0] = 0;

	// Currently raw crash data stored in `uctx` can be utilized for stalk walking on Windows only
	void* ProgramCounter = uctx->exception_ptrs.ExceptionRecord->ExceptionAddress;

	FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, ProgramCounter);

	FCString::Strncat(GErrorHist, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, TEXT("\r\n\r\n"), UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, ANSI_TO_TCHAR(StackTrace), UE_ARRAY_COUNT(GErrorHist));

#if !NO_LOGGING
	FDebug::LogFormattedMessageWithCallstack(LogSentrySdk.GetCategoryName(), __FILE__, __LINE__, TEXT("=== Critical error: ==="), GErrorHist, ELogVerbosity::Error);
#endif // !NO_LOGGING

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	GLog->Panic();
#endif // !UE_VERSION_OLDER_THAN(5, 1, 0)

	GMalloc->Free(StackTrace);
#endif // !UE_VERSION_OLDER_THAN(5, 0, 0)
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
{
	// Ensures that error message and corresponding callstack flushed to a log file (if available)
	// before it's attached to the captured crash event and uploaded to Sentry.
	PrintCrashLog(uctx);

	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
