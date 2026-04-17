// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryHangTest.h"

#include "SentryPlayground/SentryPlayground.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentrySubsystem.h"

#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersionComparison.h"

void FSentryHangTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Flush logs to ensure output is captured before hang
	GLog->Flush();

	Subsystem->SetTag(TEXT("test.hang_id"), EventId);

	// Defer the hang trigger until after engine loop init so that the heartbeat
	// monitor thread is running and threads have registered their heartbeats.
	// Capture AsShared() so this test stays alive past RunIntegrationTest returning.
	TSharedRef<FSentryBaseIntegrationTest> Self = AsShared();
	FCoreDelegates::OnFEngineLoopInitComplete.AddLambda([Self]()
	{
		USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType::Hang);

		Self->GetSubsystem()->Close();
		Self->CompleteWithResult(true);
	});
}
