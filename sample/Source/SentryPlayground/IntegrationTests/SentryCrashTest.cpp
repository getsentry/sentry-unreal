// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryCrashTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/EngineVersionComparison.h"

void FSentryCrashTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	// Because we don't get the real crash event ID, create a fake one and set it as a tag
	// This tag is then used by integration test script in CI to fetch the event

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	if (FSentryModule::Get().GetSettings()->UseNativeBackend)
	{
		// When using native backend, crashing process can terminate before engine's GLog is flushed
		// To ensure the event ID is not lost it is printed directly to stdout so the test script can reliably capture it
		FPlatformMisc::LowLevelOutputDebugStringf(TEXT("EVENT_CAPTURED: %s\n"), *EventId);
	}
	else
	{
		UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
	}
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Flush logs to ensure output is captured before crash
	GLog->Flush();

	Subsystem->SetTag(TEXT("test.crash_id"), EventId);

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	USentryPlaygroundCrashUtils::Terminate(CrashType);
}
