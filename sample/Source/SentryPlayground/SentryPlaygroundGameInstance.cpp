// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundGameInstance.h"

#include "SentryPlayground.h"
#include "SentrySubsystem.h"
#include "SentrySettings.h"
#include "SentryPlaygroundUtils.h"
#include "SentryUser.h"

#include "Misc/CommandLine.h"
#include "Engine/Engine.h"

void USentryPlaygroundGameInstance::Init()
{
	Super::Init();

	const TCHAR* CommandLine = FCommandLine::Get();

	// Check for expected test parameters to decide between running integration tests
	// or launching the sample app with UI for manual testing
	if (FParse::Param(FCommandLine::Get(), TEXT("crash-capture")) ||
		FParse::Param(FCommandLine::Get(), TEXT("message-capture")))
	{
		RunIntegrationTest(CommandLine);
	}
	else
	{
		// Normal game startup - run diagnostics after Sentry initialization
		USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
		if (SentrySubsystem && SentrySubsystem->IsEnabled())
		{
			UE_LOG(LogSentrySample, Log, TEXT("========================================"));
			UE_LOG(LogSentrySample, Log, TEXT("Running Crashpad diagnostics on startup..."));
			UE_LOG(LogSentrySample, Log, TEXT("========================================"));

			USentryPlaygroundUtils::LogCrashpadDiagnostics();

			UE_LOG(LogSentrySample, Log, TEXT("========================================"));
			UE_LOG(LogSentrySample, Log, TEXT("Crashpad diagnostics complete"));
			UE_LOG(LogSentrySample, Log, TEXT("========================================"));
		}
	}
}

void USentryPlaygroundGameInstance::RunIntegrationTest(const TCHAR* CommandLine)
{
	UE_LOG(LogSentrySample, Log, TEXT("Running integration test for command: %s\n"), CommandLine);

	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!SentrySubsystem)
	{
		CompleteTestWithResult(TEXT("sentry-error"), false, TEXT("Invalid Sentry subsystem"));
		return;
	}

	SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
	{
		// Override options set in config file if needed
		FString Dsn;
		if (FParse::Value(CommandLine, TEXT("dsn="), Dsn))
		{
			Settings->Dsn = Dsn;
		}
	}));

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

	if (FParse::Param(CommandLine, TEXT("crash-capture")))
	{
		RunCrashTest();
	}
	else if (FParse::Param(CommandLine, TEXT("message-capture")))
	{
		RunMessageTest();
	}
}

void USentryPlaygroundGameInstance::RunCrashTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	// Because we don't get the real crash event ID, create a fake one and set it as a tag
	// This tag is then used by integration test script in CI to fetch the event

	FString EventId = FGuid::NewGuid().ToString(EGuidFormats::Digits);

	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);

	SentrySubsystem->SetTag(TEXT("test.crash_id"), EventId);

	USentryPlaygroundUtils::Terminate(ESentryAppTerminationType::NullPointer);
}

void USentryPlaygroundGameInstance::RunMessageTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	FString EventId = SentrySubsystem->CaptureMessage(TEXT("Integration test message"));

	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);

	CompleteTestWithResult(TEXT("message-capture"), !EventId.IsEmpty(), TEXT("Test complete"));
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
}

void USentryPlaygroundGameInstance::CompleteTestWithResult(const FString& TestName, bool Result, const FString& Message)
{
	UE_LOG(LogSentrySample, Log, TEXT("TEST_RESULT: {\"test\":\"%s\",\"success\":%s,\"message\":\"%s\"}\n"),
		*TestName, Result ? TEXT("true") : TEXT("false"), *Message);

	// Close app after test is completed
	FGenericPlatformMisc::RequestExit(false);
}
