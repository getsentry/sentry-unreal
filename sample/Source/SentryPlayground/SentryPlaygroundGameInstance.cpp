// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundGameInstance.h"

#include "SentryPlayground.h"
#include "SentrySubsystem.h"
#include "SentrySettings.h"
#include "SentryPlaygroundUtils.h"
#include "SentryBreadcrumb.h"
#include "SentryScope.h"
#include "SentryUser.h"
#include "SentryUnit.h"
#include "SentryVariant.h"

#include "CoreGlobals.h"
#include "HAL/Platform.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/CommandLine.h"
#include "Engine/Engine.h"

void USentryPlaygroundGameInstance::Init()
{
	Super::Init();

	FString CommandLine = FCommandLine::Get();

	UE_LOG(LogSentrySample, Display, TEXT("Starting app with commandline: %s\n"), *CommandLine);

	// Check for expected test parameters to decide between running integration tests
	// or launching the sample app with UI for manual testing
	if (FParse::Param(*CommandLine, TEXT("crash-capture")) ||
		FParse::Param(*CommandLine, TEXT("crash-stack-overflow")) ||
		FParse::Param(*CommandLine, TEXT("crash-memory-corruption")) ||
		FParse::Param(*CommandLine, TEXT("message-capture")) ||
		FParse::Param(*CommandLine, TEXT("log-capture")) ||
		FParse::Param(*CommandLine, TEXT("metric-capture")) ||
		FParse::Param(*CommandLine, TEXT("init-only")))
	{
		RunIntegrationTest(CommandLine);
	}
}

void USentryPlaygroundGameInstance::RunIntegrationTest(const FString& CommandLine)
{
	UE_LOG(LogSentrySample, Display, TEXT("Running integration test for command: %s\n"), *CommandLine);

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!SentrySubsystem)
	{
		CompleteTestWithResult(TEXT("sentry-error"), false, TEXT("Invalid Sentry subsystem"));
		return;
	}

	if (!SentrySubsystem->IsEnabled())
	{
		CompleteTestWithResult(TEXT("sentry-error"), false, TEXT("Failed to initialize Sentry"));
		return;
	}

	SentrySubsystem->AddBreadcrumbWithParams(
		TEXT("Integration test started"), TEXT("Test"), TEXT("info"), TMap<FString, FSentryVariant>(), ESentryLevel::Info);

	ConfigureTestContext();

	SentrySubsystem->AddBreadcrumbWithParams(
		TEXT("Context configuration finished"), TEXT("Test"), TEXT("info"), TMap<FString, FSentryVariant>(), ESentryLevel::Info);

	if (FParse::Param(*CommandLine, TEXT("crash-capture")))
	{
		RunCrashTest(ESentryAppTerminationType::NullPointer);
	}
	else if (FParse::Param(*CommandLine, TEXT("crash-stack-overflow")))
	{
		RunCrashTest(ESentryAppTerminationType::StackOverflow);
	}
	else if (FParse::Param(*CommandLine, TEXT("crash-memory-corruption")))
	{
		RunCrashTest(ESentryAppTerminationType::MemoryCorruption);
	}
	else if (FParse::Param(*CommandLine, TEXT("message-capture")))
	{
		RunMessageTest();
	}
	else if (FParse::Param(*CommandLine, TEXT("log-capture")))
	{
		RunLogTest();
	}
	else if (FParse::Param(*CommandLine, TEXT("metric-capture")))
	{
		RunMetricTest();
	}
	else if (FParse::Param(*CommandLine, TEXT("init-only")))
	{
		RunInitOnly();
	}
}

void USentryPlaygroundGameInstance::RunCrashTest(ESentryAppTerminationType CrashType)
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	// Because we don't get the real crash event ID, create a fake one and set it as a tag
	// This tag is then used by integration test script in CI to fetch the event

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *EventId);
#endif

	// Flush logs to ensure output is captured before crash
	GLog->Flush();

	SentrySubsystem->SetTag(TEXT("test.crash_id"), EventId);

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	USentryPlaygroundUtils::Terminate(CrashType);
}

void USentryPlaygroundGameInstance::RunMessageTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	FString EventId = SentrySubsystem->CaptureMessageWithScope(TEXT("Integration test message"), FConfigureScopeNativeDelegate::CreateLambda([](USentryScope* Scope)
		{
			// Local scope tag
			Scope->SetTag(TEXT("scope.locality"), TEXT("local"));

			// Local scope extras (one persists, one for beforeSend to remove)
			Scope->SetExtra(TEXT("local_extra"), FSentryVariant(TEXT("local_extra_value")));
			Scope->SetExtra(TEXT("extra_to_be_removed"), FSentryVariant(TEXT("original_value")));

			// Local scope context
			TMap<FString, FSentryVariant> LocalContext;
			LocalContext.Add(TEXT("local_key"), FSentryVariant(TEXT("local_value")));
			Scope->SetContext(TEXT("local_context"), LocalContext);

			// Local scope breadcrumb
			USentryBreadcrumb* Breadcrumb = NewObject<USentryBreadcrumb>();
			Breadcrumb->Initialize();
			Breadcrumb->SetMessage(TEXT("Local scope breadcrumb"));
			Breadcrumb->SetCategory(TEXT("test"));
			Breadcrumb->SetType(TEXT("info"));
			Scope->AddBreadcrumb(Breadcrumb);
		}),
		ESentryLevel::Info);

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#endif

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	SentrySubsystem->Close();

	CompleteTestWithResult(TEXT("message-capture"), !EventId.IsEmpty(), TEXT("Test complete"));
}

void USentryPlaygroundGameInstance::RunLogTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	const FString LogMessage = TEXT("Integration test structured log");
	const FString LogCategory = TEXT("LogSentryTest");

	FString TestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	SentrySubsystem->SetAttribute(TEXT("global_attr"), FSentryVariant(TEXT("global_value")));

	SentrySubsystem->SetAttribute(TEXT("global_removed"), FSentryVariant(TEXT("should_not_appear")));
	SentrySubsystem->RemoveAttribute(TEXT("global_removed"));

	TMap<FString, FSentryVariant> Attributes;
	Attributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	Attributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	SentrySubsystem->LogWarningWithAttributes(LogMessage, Attributes, LogCategory);

	UE_LOG(LogSentrySample, Display, TEXT("LOG_TRIGGERED: %s\n"), *TestId);

	// Ensure events were flushed
	SentrySubsystem->Close();

	FPlatformProcess::Sleep(1.0f);

	CompleteTestWithResult(TEXT("log-capture"), true, TEXT("Test complete"));
}

void USentryPlaygroundGameInstance::RunMetricTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	FString TestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	TMap<FString, FSentryVariant> CounterAttributes;
	CounterAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	CounterAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	TMap<FString, FSentryVariant> DistributionAttributes;
	DistributionAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	DistributionAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	TMap<FString, FSentryVariant> GaugeAttributes;
	GaugeAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	GaugeAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	SentrySubsystem->AddCountWithAttributes(TEXT("test.integration.counter"), 1, CounterAttributes);
	SentrySubsystem->AddDistributionWithAttributes(TEXT("test.integration.distribution"), 42.5f, FSentryUnit(ESentryUnit::Millisecond), DistributionAttributes);
	SentrySubsystem->AddGaugeWithAttributes(TEXT("test.integration.gauge"), 15.0f, FSentryUnit(ESentryUnit::Byte), GaugeAttributes);

	UE_LOG(LogSentrySample, Display, TEXT("METRIC_TRIGGERED: %s\n"), *TestId);

	// Ensure events were flushed
	SentrySubsystem->Close();

	FPlatformProcess::Sleep(1.0f);

	CompleteTestWithResult(TEXT("metric-capture"), true, TEXT("Test complete"));
}

void USentryPlaygroundGameInstance::RunInitOnly()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

#if PLATFORM_ANDROID
	FPlatformProcess::Sleep(1.0f);
#endif

	// Ensure events were flushed
	SentrySubsystem->Close();

	CompleteTestWithResult(TEXT("init-only"), true, TEXT("Test complete"));
}

void USentryPlaygroundGameInstance::ConfigureTestContext()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	USentryUser* User = NewObject<USentryUser>();
	User->Initialize();
	User->SetUsername(TEXT("TestUser"));
	User->SetEmail(TEXT("user-mail@test.abc"));
	User->SetId(TEXT("12345"));

	SentrySubsystem->SetUser(User);

	SentrySubsystem->SetTag(TEXT("test.suite"), TEXT("integration"));

	// Tag to be removed by beforeSend handler
	SentrySubsystem->SetTag(TEXT("tag_to_be_removed"), TEXT("original_value"));

	// Global context
	TMap<FString, FSentryVariant> TestContext;
	TestContext.Add(TEXT("context_key"), FSentryVariant(TEXT("context_value")));
	SentrySubsystem->SetContext(TEXT("test_context"), TestContext);

	// Context to be removed by beforeSend handler
	TMap<FString, FSentryVariant> ContextRemovedByHandler;
	ContextRemovedByHandler.Add(TEXT("key"), FSentryVariant(TEXT("original_value")));
	SentrySubsystem->SetContext(TEXT("context_removed_by_handler"), ContextRemovedByHandler);

}

void USentryPlaygroundGameInstance::CompleteTestWithResult(const FString& TestName, bool Result, const FString& Message)
{
	UE_LOG(LogSentrySample, Display, TEXT("TEST_RESULT: {\"test\":\"%s\",\"success\":%s,\"message\":\"%s\"}\n"),
		*TestName, Result ? TEXT("true") : TEXT("false"), *Message);

	// Flush logs to ensure output is captured before exit
	GLog->Flush();

	// Close app after test is completed
	FPlatformMisc::RequestExitWithStatus(true, 0);
}

FString USentryPlaygroundGameInstance::FormatEventIdWithHyphens(const FString& EventId)
{
	if (EventId.Len() == 32 && !EventId.Contains(TEXT("-")))
	{
		return FString::Printf(TEXT("%s-%s-%s-%s-%s"),
			*EventId.Mid(0, 8),
			*EventId.Mid(8, 4),
			*EventId.Mid(12, 4),
			*EventId.Mid(16, 4),
			*EventId.Mid(20, 12));
	}

	return EventId;
}
