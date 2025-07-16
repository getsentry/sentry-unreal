// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryScope.h"
#include "SentryEvent.h"
#include "SentryScopeBeforeSendHandler.h"
#include "SentryTests.h"

#include "Engine/Engine.h"
#include "SentryDataTypes.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryEvent.h"
#include "HAL/PlatformSentryScope.h"

TDelegate<void(USentryEvent*)> UScopeTestBeforeSendHandler::OnScopeTestBeforeSendHandler;

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryScopeSpec, "Sentry.SentryScope", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryScope* SentryScope;
	FString TestDist;
	FString TestEnvironment;
	TMap<FString, FString> TestTags;
	TMap<FString, FSentryVariant> TestExtras;
	TMap<FString, FSentryVariant> TestContext;
	TArray<FString> TestFingerprint;
END_DEFINE_SPEC(SentryScopeSpec)

void SentryScopeSpec::Define()
{
	BeforeEach([this]()
	{
		SentryScope = USentryScope::Create(MakeShareable(new FPlatformSentryScope));

		TestTags.Add(TEXT("TagsKey1"), TEXT("TagsVal1"));
		TestTags.Add(TEXT("TagsKey2"), TEXT("TagsVal2"));

		TestExtras.Add(TEXT("ExtrasKey1"), TEXT("ExtrasVal1"));
		TestExtras.Add(TEXT("ExtrasKey2"), TEXT("ExtrasVal2"));

		TestContext.Add(TEXT("ContextKey1"), TEXT("ContextVal1"));
		TestContext.Add(TEXT("ContextKey2"), TEXT("ContextVal2"));

		TestFingerprint.Add(TEXT("F1"));
		TestFingerprint.Add(TEXT("F2"));
		TestFingerprint.Add(TEXT("F3"));
	});

	Describe("Scope tags", [this]()
	{
		It("should persist value when single item set", [this]()
		{
			SentryScope->SetTag(TEXT("Key1"), TEXT("Val1"));

			TestEqual("Tag exists", SentryScope->GetTag(TEXT("Key1")), TEXT("Val1"));
		});

		It("should persist value when multiple items set", [this]()
		{
			SentryScope->SetTags(TestTags);

			TMap<FString, FString> ReceivedTags = SentryScope->GetTags();
			TestEqual("Tag 1 exists", ReceivedTags[TEXT("TagsKey1")], TestTags[TEXT("TagsKey1")]);
			TestEqual("Tag 2 exists", ReceivedTags[TEXT("TagsKey2")], TestTags[TEXT("TagsKey2")]);
		});

		It("should be removable", [this]()
		{
			SentryScope->SetTag(TEXT("Key1"), TEXT("Val1"));

			SentryScope->RemoveTag(TEXT("Key1"));

			TestEqual("Tag removed", SentryScope->GetTag(TEXT("Key1")), TEXT(""));
		});
	});

	Describe("Scope contexts", [this]()
	{
		It("should persist their values", [this]()
		{
			TMap<FString, FSentryVariant> NonExistingContext;
			TestFalse("No context with given key available before it was added", SentryScope->TryGetContext(TEXT("TestContext"), NonExistingContext));

			SentryScope->SetContext(TEXT("TestContext"), TestContext);

			TMap<FString, FSentryVariant> RetrievedContext = SentryScope->GetContext(TEXT("TestContext"));

			TestEqual("Scope context retains all values", RetrievedContext.Num(), 2);
			TestEqual("Scope context 1", RetrievedContext[TEXT("ContextKey1")], TestContext[TEXT("ContextKey1")]);
			TestEqual("Scope context 2", RetrievedContext[TEXT("ContextKey2")], TestContext[TEXT("ContextKey2")]);

			SentryScope->RemoveContext(TEXT("TestContext"));
			TestFalse("No context with given key available after it was removed", SentryScope->TryGetContext(TEXT("TestContext"), NonExistingContext));
		});
	});

	Describe("Scope extras", [this]()
	{
		It("should persist value when single item set", [this]()
		{
			SentryScope->SetExtra(TEXT("Key1"), TEXT("Val1"));

			TestEqual("Extra value exists", SentryScope->GetExtra(TEXT("Key1")).GetValue<FString>(), TEXT("Val1"));
		});

		It("should persist value when multiple items set", [this]()
		{
			SentryScope->SetExtras(TestExtras);

			TMap<FString, FSentryVariant> ReceivedExtras = SentryScope->GetExtras();
			TestEqual("Extra 1 exists", ReceivedExtras[TEXT("ExtrasKey1")], TestExtras[TEXT("ExtrasKey1")]);
			TestEqual("Extra 2 exists", ReceivedExtras[TEXT("ExtrasKey2")], TestExtras[TEXT("ExtrasKey2")]);
		});

		It("should be removable", [this]()
		{
			SentryScope->SetTag(TEXT("Key1"), TEXT("Val1"));

			SentryScope->RemoveTag(TEXT("Key1"));

			TestEqual("Extra value removed", SentryScope->GetTag(TEXT("Key1")), TEXT(""));
		});
	});

	Describe("Scope params", [this]()
	{
		It("should persist their values", [this]()
		{
			SentryScope->SetLevel(ESentryLevel::Fatal);
			SentryScope->SetFingerprint(TestFingerprint);

			TestEqual("Scope level", SentryScope->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Scope fingerprint", SentryScope->GetFingerprint(), TestFingerprint);
		});
	});

	Describe("Scope params", [this]()
	{
		It("should be possible to clear", [this]()
		{
			SentryScope->SetFingerprint(TestFingerprint);
			SentryScope->SetTags(TestTags);
			SentryScope->SetExtras(TestExtras);

			SentryScope->Clear();

			TestTrue("Scope fingerprint", SentryScope->GetFingerprint().Num() == 0);
			TestTrue("Scope tags", SentryScope->GetTags().Num() == 0);
			TestTrue("Scope extras", SentryScope->GetExtras().Num() == 0);
		});
	});

	Describe("Scope params", [this]()
	{
		It("should be applied to event", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
			SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
			{
				Settings->BeforeSendHandler = UScopeTestBeforeSendHandler::StaticClass();
			}));

			UScopeTestBeforeSendHandler::OnScopeTestBeforeSendHandler.BindLambda([this](USentryEvent* SentryEvent)
			{
				TestEqual("Event level", SentryEvent->GetLevel(), ESentryLevel::Fatal);
				TestEqual("Event fingerprint", SentryEvent->GetFingerprint(), TestFingerprint);
				TestEqual("Event tags 1", SentryEvent->GetTags()[TEXT("TagsKey1")], TestTags[TEXT("TagsKey1")]);
				TestEqual("Event tags 2", SentryEvent->GetTags()[TEXT("TagsKey2")], TestTags[TEXT("TagsKey2")]);
				TestEqual("Event extra 1", SentryEvent->GetExtras()[TEXT("ExtrasKey1")], TestExtras[TEXT("ExtrasKey1")]);
				TestEqual("Event extra 2", SentryEvent->GetExtras()[TEXT("ExtrasKey2")], TestExtras[TEXT("ExtrasKey2")]);
				TestEqual("Event context 1", SentryEvent->GetContext(TEXT("TestContext"))[TEXT("ContextKey1")], TestContext[TEXT("ContextKey1")]);
				TestEqual("Event context 2", SentryEvent->GetContext(TEXT("TestContext"))[TEXT("ContextKey2")], TestContext[TEXT("ContextKey2")]);
			});

			USentryEvent* SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());
			SentrySubsystem->CaptureEventWithScope(SentryEvent, FConfigureScopeNativeDelegate::CreateLambda([this](USentryScope* Scope)
			{
				Scope->SetLevel(ESentryLevel::Fatal);
				Scope->SetFingerprint(TestFingerprint);
				Scope->SetTags(TestTags);
				Scope->SetExtras(TestExtras);
				Scope->SetContext(TEXT("TestContext"), TestContext);
			}));

			UScopeTestBeforeSendHandler::OnScopeTestBeforeSendHandler.Unbind();

			SentrySubsystem->Close();
		});
	});
}

#endif
