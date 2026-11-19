// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryLogTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentrySubsystem.h"
#include "SentryVariant.h"

#include "HAL/PlatformProcess.h"

void FSentryLogTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	const FString LogMessage = TEXT("Integration test structured log metadata=\"%s\" (%d fields)");
	const FString LogCategory = TEXT("LogSentryTest");

	FString TestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	Subsystem->SetAttribute(TEXT("global_attr"), FSentryVariant(TEXT("global_value")));

	Subsystem->SetAttribute(TEXT("global_removed"), FSentryVariant(TEXT("should_not_appear")));
	Subsystem->RemoveAttribute(TEXT("global_removed"));

	TMap<FString, FSentryVariant> Attributes;
	Attributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	Attributes.Add(TEXT("log_kind"), FSentryVariant(TEXT("attributes")));
	Attributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	Subsystem->LogWarningWithAttributes(LogMessage, Attributes, LogCategory);

	TMap<FString, FSentryVariant> TraceAttributes;
	TraceAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	TraceAttributes.Add(TEXT("log_kind"), FSentryVariant(TEXT("trace")));

	Subsystem->LogTraceWithAttributes(TEXT("Integration test trace log"), TraceAttributes, LogCategory);

	// Each log is sent with one level and changed to the level in `level_override` by UCppBeforeLogHandler
	struct FLevelOverride
	{
		void (USentrySubsystem::*Log)(const FString&, const TMap<FString, FSentryVariant>&, const FString&);
		const TCHAR* Level;
	};

	const FLevelOverride LevelOverrides[] = {
		{ &USentrySubsystem::LogTraceWithAttributes, TEXT("Fatal") },
		{ &USentrySubsystem::LogDebugWithAttributes, TEXT("Trace") },
		{ &USentrySubsystem::LogInfoWithAttributes, TEXT("Debug") },
		{ &USentrySubsystem::LogWarningWithAttributes, TEXT("Info") },
		{ &USentrySubsystem::LogErrorWithAttributes, TEXT("Warning") },
		{ &USentrySubsystem::LogFatalWithAttributes, TEXT("Error") }
	};

	for (const FLevelOverride& LevelOverride : LevelOverrides)
	{
		TMap<FString, FSentryVariant> OverrideAttributes;
		OverrideAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
		OverrideAttributes.Add(TEXT("log_kind"), FSentryVariant(TEXT("level_override")));
		OverrideAttributes.Add(TEXT("level_override"), FSentryVariant(LevelOverride.Level));

		(Subsystem->*LevelOverride.Log)(TEXT("Integration test level override log"), OverrideAttributes, LogCategory);
	}

	UE_LOG(LogSentrySample, Display, TEXT("LOG_TRIGGERED: %s\n"), *TestId);

	// Ensure events were flushed
	Subsystem->Close();

	FPlatformProcess::Sleep(1.0f);

	CompleteWithResult(true);
}
