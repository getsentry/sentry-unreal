// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundGameInstance.h"

#include "SentryPlayground.h"
#include "SentrySubsystem.h"
#include "SentrySettings.h"
#include "SentryPlaygroundUtils.h"
#include "SentryUser.h"

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
		FParse::Param(*CommandLine, TEXT("crash-stackoverflow")) ||
		FParse::Param(*CommandLine, TEXT("crash-memorycorruption")) ||
		FParse::Param(*CommandLine, TEXT("message-capture")) ||
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

	SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([CommandLine](USentrySettings* Settings)
	{
		// Override options set in config file if needed
		FString Dsn;
		if (FParse::Value(*CommandLine, TEXT("dsn="), Dsn))
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

	if (FParse::Param(*CommandLine, TEXT("crash-capture")))
	{
		RunCrashTest(ESentryAppTerminationType::NullPointer);
	}
	else if (FParse::Param(*CommandLine, TEXT("crash-stackoverflow")))
	{
		RunCrashTest(ESentryAppTerminationType::StackOverflow);
	}
	else if (FParse::Param(*CommandLine, TEXT("crash-memorycorruption")))
	{
		RunCrashTest(ESentryAppTerminationType::MemoryCorruption);
	}
	else if (FParse::Param(*CommandLine, TEXT("message-capture")))
	{
		RunMessageTest();
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

	FString EventId = SentrySubsystem->CaptureMessage(TEXT("Integration test message"));

	// Workaround for duplicated log messages in UE 4.27 on Linux
#if PLATFORM_LINUX && UE_VERSION_OLDER_THAN(5, 0, 0)
	UE_LOG(LogSentrySample, Log, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#else
	UE_LOG(LogSentrySample, Display, TEXT("EVENT_CAPTURED: %s\n"), *FormatEventIdWithHyphens(EventId));
#endif

	// Ensure events were flushed
	SentrySubsystem->Close();

	CompleteTestWithResult(TEXT("message-capture"), !EventId.IsEmpty(), TEXT("Test complete"));
}

void USentryPlaygroundGameInstance::RunInitOnly()
{
	USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

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
