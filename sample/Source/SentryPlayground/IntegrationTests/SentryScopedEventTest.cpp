// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryScopedEventTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentryEvent.h"
#include "SentryScope.h"
#include "SentrySubsystem.h"

#include "Misc/EngineVersionComparison.h"

void FSentryScopedEventTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	USentryEvent* Event = NewObject<USentryEvent>();
	Event->Initialize();
	Event->SetMessage(TEXT("Integration test scoped event"));

	// Local scope intentionally doesn't set a level
	FString EventId = Subsystem->CaptureEventWithScope(Event, FConfigureScopeNativeDelegate::CreateLambda([](USentryScope* Scope)
		{
			Scope->SetTag(TEXT("scope.locality"), TEXT("local"));
		}));

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#endif

	// Ensure events were flushed
	Subsystem->Close();

	CompleteWithResult(!EventId.IsEmpty());
}
