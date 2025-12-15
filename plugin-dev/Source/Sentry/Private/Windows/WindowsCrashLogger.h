// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "CoreMinimal.h"

#include "GenericPlatform/Convenience/GenericPlatformSentryInclude.h"

/**
 * Class that manages a dedicated thread for safe crash logging on Windows.
 *
 * This class creates a separate thread that performs stack walking and logging
 * during crash handling. By using a separate thread, we:
 * - Avoid stack overflow issues (thread has its own stack)
 * - Can safely walk the crashed thread's stack
 * - Write crash information to game log file without risking secondary crashes
 *
 * The implementation mirrors Unreal's own Crash Reporter design (see WindowsPlatformCrashContext.cpp).
 */
class FWindowsCrashLogger
{
public:
	FWindowsCrashLogger();
	~FWindowsCrashLogger();

	/**
	 * Logs crash information from a separate thread.
	 * This is the main entry point called from the OnCrash hook.
	 *
	 * @param CrashContext - Sentry crash context containing exception information
	 * @param CrashedThreadHandle - Handle to the crashed thread (for cross-thread stack walking)
	 * @param TimeoutMs - Maximum time to wait for logging to complete (default: 5 seconds)
	 * @return true if logging completed successfully, false if timeout or error
	 */
	bool LogCrash(const sentry_ucontext_t* CrashContext, HANDLE CrashedThreadHandle, DWORD TimeoutMs = 5000);

	/**
	 * Checks if the crash logger thread is running.
	 */
	bool IsThreadRunning() const { return CrashLoggingThread != nullptr; }

private:
	/**
	 * Thread procedure that waits for crash events and performs logging.
	 */
	static DWORD WINAPI CrashLoggingThreadProc(LPVOID Parameter);

	/**
	 * Performs the actual crash logging work (called from the logging thread).
	 */
	void PerformCrashLogging();

	/**
	 * Writes exception information and stack trace to GErrorHist.
	 * Uses only pre-allocated buffers and crash-safe functions.
	 */
	void WriteToErrorBuffers(const sentry_ucontext_t* CrashContext, HANDLE CrashedThreadHandle);

	/**
	 * Safely retrieves the exception address from crash context.
	 * Returns nullptr if the exception record is null or invalid.
	 */
	void* GetExceptionAddress(const sentry_ucontext_t* CrashContext);

private:
	// Thread handle
	HANDLE CrashLoggingThread;
	DWORD CrashLoggingThreadId;

	// Synchronization events
	HANDLE CrashEvent;			// Signaled when a crash occurs
	HANDLE CrashCompletedEvent; // Signaled when logging is complete
	HANDLE StopThreadEvent;		// Signaled to stop the thread

	// Shared data (accessed from both main thread and logging thread)
	// These must be set before signaling CrashEvent
	const sentry_ucontext_t* SharedCrashContext;
	HANDLE SharedCrashedThreadHandle;
};

#endif // USE_SENTRY_NATIVE
