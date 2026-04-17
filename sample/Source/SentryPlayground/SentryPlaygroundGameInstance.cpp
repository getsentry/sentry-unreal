// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundGameInstance.h"

#include "SentryPlayground.h"

#include "SentryPlayground/IntegrationTests/SentryCrashTest.h"
#include "SentryPlayground/IntegrationTests/SentryEnsureTest.h"
#include "SentryPlayground/IntegrationTests/SentryHangTest.h"
#include "SentryPlayground/IntegrationTests/SentryInitOnlyTest.h"
#include "SentryPlayground/IntegrationTests/SentryBaseIntegrationTest.h"
#include "SentryPlayground/IntegrationTests/SentryLogTest.h"
#include "SentryPlayground/IntegrationTests/SentryMessageTest.h"
#include "SentryPlayground/IntegrationTests/SentryMetricTest.h"
#include "SentryPlayground/IntegrationTests/SentryTracingTest.h"
#include "SentryPlayground/Utils/SentryPlaygroundCrashUtils.h"

#include "SentrySubsystem.h"
#include "SentryUser.h"
#include "SentryVariant.h"

#include "Engine/Engine.h"
#include "Misc/CommandLine.h"

void USentryPlaygroundGameInstance::Init()
{
	Super::Init();

	FString CommandLine = FCommandLine::Get();

	UE_LOG(LogSentrySample, Display, TEXT("Starting app with commandline: %s\n"), *CommandLine);

	if (TSharedPtr<FSentryBaseIntegrationTest> Test = CheckForPendingIntegrationTest(CommandLine))
	{
		RunIntegrationTest(Test.ToSharedRef());
	}
}

TSharedPtr<FSentryBaseIntegrationTest> USentryPlaygroundGameInstance::CheckForPendingIntegrationTest(const FString& CommandLine) const
{
	const TArray<FIntegrationTestEntry> Entries =
	{
		{ TEXT("crash-capture"),           []{ return MakeShared<FSentryCrashTest>(TEXT("crash-capture"),           ESentryAppTerminationType::NullPointer); } },
		{ TEXT("crash-stack-overflow"),    []{ return MakeShared<FSentryCrashTest>(TEXT("crash-stack-overflow"),    ESentryAppTerminationType::StackOverflow); } },
		{ TEXT("crash-memory-corruption"), []{ return MakeShared<FSentryCrashTest>(TEXT("crash-memory-corruption"), ESentryAppTerminationType::MemoryCorruption); } },
		{ TEXT("crash-assert"),            []{ return MakeShared<FSentryCrashTest>(TEXT("crash-assert"),            ESentryAppTerminationType::Assert); } },
		{ TEXT("crash-oom"),               []{ return MakeShared<FSentryCrashTest>(TEXT("crash-oom"),               ESentryAppTerminationType::OutOfMemory); } },
		{ TEXT("message-capture"),         []{ return MakeShared<FSentryMessageTest>(); } },
		{ TEXT("log-capture"),             []{ return MakeShared<FSentryLogTest>(); } },
		{ TEXT("metric-capture"),          []{ return MakeShared<FSentryMetricTest>(); } },
		{ TEXT("tracing-capture"),         []{ return MakeShared<FSentryTracingTest>(); } },
		{ TEXT("ensure-capture"),          []{ return MakeShared<FSentryEnsureTest>(); } },
		{ TEXT("hang-capture"),            []{ return MakeShared<FSentryHangTest>(); } },
		{ TEXT("init-only"),               []{ return MakeShared<FSentryInitOnlyTest>(); } },
	};

	for (const FIntegrationTestEntry& Entry : Entries)
	{
		if (FParse::Param(*CommandLine, Entry.Param))
		{
			return Entry.MakeTest();
		}
	}

	return nullptr;
}

void USentryPlaygroundGameInstance::RunIntegrationTest(TSharedRef<FSentryBaseIntegrationTest> Test)
{
	UE_LOG(LogSentrySample, Display, TEXT("Running integration test: %s\n"), *Test->GetName());

	USentrySubsystem* Subsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
	if (!Subsystem)
	{
		Test->CompleteWithResult(false, TEXT("Invalid Sentry subsystem"));
		return;
	}

	if (!Subsystem->IsEnabled())
	{
		Test->CompleteWithResult(false, TEXT("Failed to initialize Sentry"));
		return;
	}

	Subsystem->AddBreadcrumbWithParams(
		TEXT("Integration test started"), TEXT("Test"), TEXT("info"), TMap<FString, FSentryVariant>(), ESentryLevel::Info);

	ConfigureTestContext(Subsystem);

	Subsystem->AddBreadcrumbWithParams(
		TEXT("Context configuration finished"), TEXT("Test"), TEXT("info"), TMap<FString, FSentryVariant>(), ESentryLevel::Info);

	Test->Run();
}

void USentryPlaygroundGameInstance::ConfigureTestContext(USentrySubsystem* Subsystem)
{
	USentryUser* User = NewObject<USentryUser>();
	User->Initialize();
	User->SetUsername(TEXT("TestUser"));
	User->SetEmail(TEXT("user-mail@test.abc"));
	User->SetId(TEXT("12345"));

	Subsystem->SetUser(User);

	Subsystem->SetTag(TEXT("test.suite"), TEXT("integration"));

	// Tag to be removed by beforeSend handler
	Subsystem->SetTag(TEXT("tag_to_be_removed"), TEXT("original_value"));

	// Global context
	TMap<FString, FSentryVariant> TestContext;
	TestContext.Add(TEXT("context_key"), FSentryVariant(TEXT("context_value")));
	Subsystem->SetContext(TEXT("test_context"), TestContext);

	// Context to be removed by beforeSend handler
	TMap<FString, FSentryVariant> ContextRemovedByHandler;
	ContextRemovedByHandler.Add(TEXT("key"), FSentryVariant(TEXT("original_value")));
	Subsystem->SetContext(TEXT("context_removed_by_handler"), ContextRemovedByHandler);

	// Breadcrumb to be discarded by beforeBreadcrumb handler
	Subsystem->AddBreadcrumbWithParams(
		TEXT("Breadcrumb to be discarded"), TEXT("Test"), TEXT("info"),
		TMap<FString, FSentryVariant>(), ESentryLevel::Info);

	// Breadcrumb to be modified by beforeBreadcrumb handler (data added by handler)
	Subsystem->AddBreadcrumbWithParams(
		TEXT("Breadcrumb to be modified"), TEXT("Test"), TEXT("info"),
		TMap<FString, FSentryVariant>(), ESentryLevel::Info);
}
