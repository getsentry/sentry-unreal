// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryFeedbackTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentryFeedback.h"
#include "SentrySubsystem.h"

#include "HAL/PlatformProcess.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Guid.h"

void FSentryFeedbackTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	Subsystem->StartSession();

	const FString EventId = Subsystem->CaptureMessage(TEXT("Integration test feedback anchor event"), ESentryLevel::Info);

	const FString Token = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString Message = FString::Printf(TEXT("Integration test feedback %s"), *Token);

	USentryFeedback* Feedback = NewObject<USentryFeedback>();
	Feedback->Initialize(Message);
	Feedback->SetName(TEXT("Feedback Test User"));
	Feedback->SetContactEmail(TEXT("feedback-user@example.com"));
	Feedback->SetAssociatedEvent(EventId);

	Subsystem->CaptureFeedback(Feedback);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
	UE_LOG(LogSentrySample, Log, TEXT("FEEDBACK_TOKEN: %s\n"), *Token);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
	UE_LOG(LogSentrySample, Display, TEXT("FEEDBACK_TOKEN: %s\n"), *Token);
#endif

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	Subsystem->Close();

	CompleteWithResult(!EventId.IsEmpty());
}
