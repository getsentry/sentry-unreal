// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryLogTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentrySubsystem.h"
#include "SentryVariant.h"

#include "HAL/PlatformProcess.h"

void FSentryLogTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	const FString LogMessage = TEXT("Integration test structured log");
	const FString LogCategory = TEXT("LogSentryTest");

	FString TestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	Subsystem->SetAttribute(TEXT("global_attr"), FSentryVariant(TEXT("global_value")));

	Subsystem->SetAttribute(TEXT("global_removed"), FSentryVariant(TEXT("should_not_appear")));
	Subsystem->RemoveAttribute(TEXT("global_removed"));

	TMap<FString, FSentryVariant> Attributes;
	Attributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	Attributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	Subsystem->LogWarningWithAttributes(LogMessage, Attributes, LogCategory);

	UE_LOG(LogSentrySample, Display, TEXT("LOG_TRIGGERED: %s\n"), *TestId);

	// Ensure events were flushed
	Subsystem->Close();

	FPlatformProcess::Sleep(1.0f);

	CompleteWithResult(true);
}
