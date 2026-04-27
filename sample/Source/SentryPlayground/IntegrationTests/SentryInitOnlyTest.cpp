// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryInitOnlyTest.h"

#include "SentrySubsystem.h"

#include "HAL/PlatformProcess.h"

void FSentryInitOnlyTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(true);
}
