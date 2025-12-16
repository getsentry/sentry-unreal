// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsCrashLogger.h"

#if USE_SENTRY_NATIVE

#include "Windows/WindowsPlatformStackWalk.h"

void* FWindowsCrashLogger::CreateContextWrapper(void* Context, HANDLE ThreadHandle)
{
	// Windows uses FWindowsPlatformStackWalk::MakeThreadContextWrapper (static function)
	return FWindowsPlatformStackWalk::MakeThreadContextWrapper(Context, ThreadHandle);
}

void FWindowsCrashLogger::ReleaseContextWrapper(void* Wrapper)
{
	// Windows uses FWindowsPlatformStackWalk::ReleaseThreadContextWrapper (static function)
	FWindowsPlatformStackWalk::ReleaseThreadContextWrapper(Wrapper);
}

#endif // USE_SENTRY_NATIVE
