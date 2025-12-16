// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "Microsoft/MicrosoftCrashLogger.h"

/**
 * Windows-specific crash logger implementation.
 *
 * Uses FWindowsPlatformStackWalk::MakeThreadContextWrapper for cross-thread stack walking.
 */
class FWindowsCrashLogger : public FMicrosoftCrashLogger
{
protected:
	virtual void* CreateContextWrapper(void* Context, HANDLE ThreadHandle) override;
	virtual void ReleaseContextWrapper(void* Wrapper) override;
};

#endif // USE_SENTRY_NATIVE
