// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftCrashLogger.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "CoreGlobals.h"
#include "HAL/PlatformStackWalk.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/OutputDeviceRedirector.h"

#include "Microsoft/AllowMicrosoftPlatformTypes.h"

FMicrosoftCrashLogger::FMicrosoftCrashLogger()
	: CrashLoggingThread(nullptr)
	, CrashLoggingThreadId(0)
	, CrashEvent(nullptr)
	, CrashCompletedEvent(nullptr)
	, StopThreadEvent(nullptr)
	, SharedCrashContext(nullptr)
	, SharedCrashedThreadHandle(nullptr)
{
	// Create synchronization events
	CrashEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);		   // Auto-reset event
	CrashCompletedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr); // Auto-reset event
	StopThreadEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);	   // Manual-reset event

	if (!CrashEvent || !CrashCompletedEvent || !StopThreadEvent)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to create crash logger synchronization events"));
		return;
	}

	// Create the crash logging thread
	CrashLoggingThread = CreateThread(
		nullptr,				// Default security attributes
		0,						// Default stack size
		CrashLoggingThreadProc, // Thread procedure
		this,					// Parameter to thread procedure
		0,						// Run immediately
		&CrashLoggingThreadId	// Thread ID output
	);

	if (CrashLoggingThread)
	{
		// Lower priority to avoid interfering with crash handling
		SetThreadPriority(CrashLoggingThread, THREAD_PRIORITY_BELOW_NORMAL);
		UE_LOG(LogSentrySdk, Log, TEXT("Crash logging thread created successfully (ID: %u)"), CrashLoggingThreadId);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to create crash logging thread"));
	}
}

FMicrosoftCrashLogger::~FMicrosoftCrashLogger()
{
	if (StopThreadEvent)
	{
		// Signal thread to stop
		SetEvent(StopThreadEvent);
	}

	if (CrashLoggingThread)
	{
		// Wait for thread to exit (with timeout)
		WaitForSingleObject(CrashLoggingThread, 1000);
		CloseHandle(CrashLoggingThread);
		CrashLoggingThread = nullptr;
	}

	// Clean up events
	if (CrashEvent)
	{
		CloseHandle(CrashEvent);
		CrashEvent = nullptr;
	}
	if (CrashCompletedEvent)
	{
		CloseHandle(CrashCompletedEvent);
		CrashCompletedEvent = nullptr;
	}
	if (StopThreadEvent)
	{
		CloseHandle(StopThreadEvent);
		StopThreadEvent = nullptr;
	}
}

bool FMicrosoftCrashLogger::LogCrash(const sentry_ucontext_t* CrashContext, HANDLE CrashedThreadHandle, DWORD TimeoutMs)
{
	if (!CrashLoggingThread || !CrashEvent || !CrashCompletedEvent)
	{
		// Thread not initialized properly
		return false;
	}

	if (!CrashContext)
	{
		// Crash context is invalid
		return false;
	}

	// Set shared data for the logging thread to access
	SharedCrashContext = CrashContext;
	SharedCrashedThreadHandle = CrashedThreadHandle;

	// Signal the logging thread that a crash occurred
	SetEvent(CrashEvent);

	// Wait for logging to complete (with timeout)
	DWORD WaitResult = WaitForSingleObject(CrashCompletedEvent, TimeoutMs);

	return (WaitResult == WAIT_OBJECT_0);
}

DWORD WINAPI FMicrosoftCrashLogger::CrashLoggingThreadProc(LPVOID Parameter)
{
	FMicrosoftCrashLogger* Logger = static_cast<FMicrosoftCrashLogger*>(Parameter);
	if (!Logger)
	{
		return 1;
	}

	HANDLE Events[2] = { Logger->CrashEvent, Logger->StopThreadEvent };

	while (true)
	{
		// Wait for either a crash event or stop event
		DWORD WaitResult = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

		if (WaitResult == WAIT_OBJECT_0)
		{
			// Crash event signaled - perform logging
			Logger->PerformCrashLogging();

			// Signal completion
			SetEvent(Logger->CrashCompletedEvent);
		}
		else if (WaitResult == WAIT_OBJECT_0 + 1)
		{
			// Stop event signaled - exit thread
			break;
		}
		else
		{
			// Error occurred
			break;
		}
	}

	return 0;
}

void FMicrosoftCrashLogger::PerformCrashLogging()
{
	// Perform stack walking and fill GErrorHist
	// This happens in a separate thread to avoid stack overflow issues
	WriteToErrorBuffers(SharedCrashContext, SharedCrashedThreadHandle);

	// NOTE: We call FDebug::LogFormattedMessageWithCallstack() and GLog->Flush() here
	// because it's unsafe to call them from the crashing thread when handling memory-related errors (e.g. stack overflow, memory corruption, etc.)

#if !NO_LOGGING
	FDebug::LogFormattedMessageWithCallstack(LogSentrySdk.GetCategoryName(), __FILE__, __LINE__, TEXT("Sentry Crash Callstack"), GErrorHist, ELogVerbosity::Error);
#endif

	if (GLog)
	{
		GLog->Flush();
	}

	// Close the crashed thread handle now that we're done with stack walking
	if (SharedCrashedThreadHandle)
	{
		CloseHandle(SharedCrashedThreadHandle);
	}

	// Clear shared data
	SharedCrashContext = nullptr;
	SharedCrashedThreadHandle = nullptr;
}

void* FMicrosoftCrashLogger::GetExceptionAddress(const sentry_ucontext_t* CrashContext)
{
	if (CrashContext && CrashContext->exception_ptrs.ExceptionRecord)
	{
		return CrashContext->exception_ptrs.ExceptionRecord->ExceptionAddress;
	}

	return nullptr;
}

void FMicrosoftCrashLogger::CrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize)
{
	EXCEPTION_RECORD* ExceptionRecord = crashContext->exception_ptrs.ExceptionRecord;
	if (!ExceptionRecord)
	{
		return;
	}

	FString ErrorString = TEXT("Unhandled Exception: ");

#define HANDLE_CASE(x)           \
	case x:                      \
		ErrorString += TEXT(#x); \
		break;

	switch (ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		ErrorString += TEXT("EXCEPTION_ACCESS_VIOLATION ");
		if (ExceptionRecord->ExceptionInformation[0] == 0)
		{
			ErrorString += TEXT("reading address ");
		}
		else if (ExceptionRecord->ExceptionInformation[0] == 1)
		{
			ErrorString += TEXT("writing address ");
		}
		ErrorString += FString::Printf(
#if PLATFORM_64BITS
			TEXT("0x%016llx"),
#else
			TEXT("0x%08x"),
#endif
			ExceptionRecord->ExceptionInformation[1]);
		break;
		HANDLE_CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
		HANDLE_CASE(EXCEPTION_DATATYPE_MISALIGNMENT)
		HANDLE_CASE(EXCEPTION_FLT_DENORMAL_OPERAND)
		HANDLE_CASE(EXCEPTION_FLT_DIVIDE_BY_ZERO)
		HANDLE_CASE(EXCEPTION_FLT_INVALID_OPERATION)
		HANDLE_CASE(EXCEPTION_ILLEGAL_INSTRUCTION)
		HANDLE_CASE(EXCEPTION_INT_DIVIDE_BY_ZERO)
		HANDLE_CASE(EXCEPTION_PRIV_INSTRUCTION)
		HANDLE_CASE(EXCEPTION_STACK_OVERFLOW)
	default:
		ErrorString += FString::Printf(TEXT("0x%08x"), (uint32)ExceptionRecord->ExceptionCode);
	}

	FCString::Strncpy(outErrorString, *ErrorString, errorStringBufSize);

#undef HANDLE_CASE
}

void FMicrosoftCrashLogger::WriteToErrorBuffers(const sentry_ucontext_t* CrashContext, HANDLE CrashedThreadHandle)
{
	// Step 1: Write exception description to GErrorExceptionDescription
	CrashContextToString(
		CrashContext,
		GErrorExceptionDescription,
		UE_ARRAY_COUNT(GErrorExceptionDescription));

	// Step 2: Append exception description to GErrorHist
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), GErrorExceptionDescription);
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), TEXT("\r\n\r\n"));
#else
	FCString::Strncat(GErrorHist, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, TEXT("\r\n\r\n"), UE_ARRAY_COUNT(GErrorHist));
#endif

	// Step 3: Perform stack walking and append to GErrorHist
	const SIZE_T StackTraceSize = 65535;

	// Allocate stack trace buffer on this thread's stack (not heap!)
	ANSICHAR* StackTrace = (ANSICHAR*)alloca(StackTraceSize);
	StackTrace[0] = 0;

	if (CrashedThreadHandle && CrashContext->exception_ptrs.ContextRecord)
	{
		// Create platform-specific context wrapper for cross-thread stack walking
		void* ContextWrapper = CreateContextWrapper(
			CrashContext->exception_ptrs.ContextRecord,
			CrashedThreadHandle);

		if (ContextWrapper)
		{
			// Platform-specific stack walking (Xbox overrides this)
			PerformStackWalk(StackTrace, StackTraceSize, ContextWrapper, CrashContext, CrashedThreadHandle);

			// Release the platform-specific context wrapper
			ReleaseContextWrapper(ContextWrapper);
		}
	}

	// Step 4: Append stack trace to GErrorHist
#if !UE_VERSION_OLDER_THAN(5, 6, 0)
	FCString::StrncatTruncateDest(GErrorHist, UE_ARRAY_COUNT(GErrorHist), ANSI_TO_TCHAR(StackTrace));
#else
	FCString::Strncat(GErrorHist, ANSI_TO_TCHAR(StackTrace), UE_ARRAY_COUNT(GErrorHist));
#endif
}

void FMicrosoftCrashLogger::PerformStackWalk(ANSICHAR* StackTrace, SIZE_T StackTraceSize, void* ContextWrapper, const sentry_ucontext_t* CrashContext, HANDLE CrashedThreadHandle)
{
	// Default implementation for Windows: use FPlatformStackWalk::StackWalkAndDump
	void* ProgramCounter = GetExceptionAddress(CrashContext);

#if !UE_VERSION_OLDER_THAN(5, 0, 0)
	FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, ProgramCounter, ContextWrapper);
#else
	FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, 0, ContextWrapper);
#endif
}

#include "Microsoft/HideMicrosoftPlatformTypes.h"

#endif // USE_SENTRY_NATIVE
