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

	// Check '-SentryIntegrationTest' commandline argument to decide between running integration tests
	// or launching the sample app with UI for manual testing
	if (FParse::Param(FCommandLine::Get(), TEXT("SentryIntegrationTest")))
	{
		RunIntegrationTest(CommandLine);
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
		if (FParse::Value(CommandLine, TEXT("Dsn="), Dsn))
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

	if (FParse::Param(CommandLine, TEXT("CrashCapture")))
	{
		RunCrashTest();
	}
	else if (FParse::Param(CommandLine, TEXT("MessageCapture")))
	{
		RunMessageTest();
	}
	else
	{
		// Integration test requested but no action specified
		CompleteTestWithResult(TEXT("invalid-command"), false, TEXT("Missing command"));
	}
}

void USentryPlaygroundGameInstance::RunCrashTest()
{
	USentryPlaygroundUtils::Terminate(ESentryAppTerminationType::NullPointer);
}

void USentryPlaygroundGameInstance::RunMessageTest()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

	FString EventId = SentrySubsystem->CaptureMessage(TEXT("Integration test message"));

	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *EventId);

	CompleteTestWithResult("message-capture", !EventId.IsEmpty(), "Test complete");
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
