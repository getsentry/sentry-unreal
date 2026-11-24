// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryEnsureTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentrySubsystem.h"

#include "Misc/EngineVersionComparison.h"

void FSentryEnsureTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	Subsystem->SetTag(TEXT("test.ensure_id"), EventId);

	// Trigger ensure - this is non-fatal and will be captured by the error output device
	USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType::Ensure);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(true);
}
