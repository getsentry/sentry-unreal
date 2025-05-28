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

#include "GenericPlatform/Infrastructure/GenericPlatformSentryConverters.h"

TDelegate<void(USentryEvent*)> UScopeTestBeforeSendHandler::OnScopeTestBeforeSendHandler;

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryScopeSpec, "Sentry.SentryScope", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryScope* SentryScope;
	FString TestDist;
	FString TestEnvironment;
	TMap<FString, FString> TestTags;
	TMap<FString, FString> TestExtras;
	TMap<FString, FString> TestContext;
	TArray<FString> TestFingerprint;
END_DEFINE_SPEC(SentryScopeSpec)

void SentryScopeSpec::Define()
{
	BeforeEach([this]()
	{
		SentryScope = USentryScope::Create(CreateSharedSentryScope());

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
			SentryScope->SetTagValue(TEXT("Key1"), TEXT("Val1"));

			TestEqual("Tag exists", SentryScope->GetTagValue(TEXT("Key1")), TEXT("Val1"));
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
			SentryScope->SetTagValue(TEXT("Key1"), TEXT("Val1"));

			SentryScope->RemoveTag(TEXT("Key1"));

			TestEqual("Tag removed", SentryScope->GetTagValue(TEXT("Key1")), TEXT(""));
		});
	});

	Describe("Scope extras", [this]()
	{
		It("should persist value when single item set", [this]()
		{
			SentryScope->SetExtraValue(TEXT("Key1"), TEXT("Val1"));

			TestEqual("Extra value exists", SentryScope->GetExtraValue(TEXT("Key1")), TEXT("Val1"));
		});

		It("should persist value when multiple items set", [this]()
		{
			SentryScope->SetExtras(TestExtras);

			TMap<FString, FString> ReceivedExtras = SentryScope->GetExtras();
			TestEqual("Extra 1 exists", ReceivedExtras[TEXT("ExtrasKey1")], TestExtras[TEXT("ExtrasKey1")]);
			TestEqual("Extra 2 exists", ReceivedExtras[TEXT("ExtrasKey2")], TestExtras[TEXT("ExtrasKey2")]);
		});

		It("should be removable", [this]()
		{
			SentryScope->SetTagValue(TEXT("Key1"), TEXT("Val1"));

			SentryScope->RemoveTag(TEXT("Key1"));

			TestEqual("Extra value removed", SentryScope->GetTagValue(TEXT("Key1")), TEXT(""));
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

#if (PLATFORM_MICROSOFT || PLATFORM_LINUX) && USE_SENTRY_NATIVE
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
				TSharedPtr<FGenericPlatformSentryEvent> Event = StaticCastSharedPtr<FGenericPlatformSentryEvent>(SentryEvent->GetNativeObject());

				sentry_value_t NativeEvent = Event->GetNativeObject();

				sentry_value_t level = sentry_value_get_by_key(NativeEvent, "level");
				sentry_value_t fingerprint = sentry_value_get_by_key(NativeEvent, "fingerprint");
				sentry_value_t tags = sentry_value_get_by_key(NativeEvent, "tags");
				sentry_value_t extra = sentry_value_get_by_key(NativeEvent, "extra");
				sentry_value_t contexts = sentry_value_get_by_key(NativeEvent, "contexts");

				sentry_value_t testContext = sentry_value_get_by_key(contexts, "TestContext");

				TestEqual("Event level", FGenericPlatformSentryConverters::SentryLevelToUnreal(level), ESentryLevel::Fatal);
				TestEqual("Event fingerprint", FGenericPlatformSentryConverters::StringArrayToUnreal(fingerprint), TestFingerprint);
				TestEqual("Event tags 1", FGenericPlatformSentryConverters::StringMapToUnreal(tags)[TEXT("TagsKey1")], TestTags[TEXT("TagsKey1")]);
				TestEqual("Event tags 2", FGenericPlatformSentryConverters::StringMapToUnreal(tags)[TEXT("TagsKey2")], TestTags[TEXT("TagsKey2")]);
				TestEqual("Event extra 1", FGenericPlatformSentryConverters::StringMapToUnreal(extra)[TEXT("ExtrasKey1")], TestExtras[TEXT("ExtrasKey1")]);
				TestEqual("Event extra 2", FGenericPlatformSentryConverters::StringMapToUnreal(extra)[TEXT("ExtrasKey2")], TestExtras[TEXT("ExtrasKey2")]);
				TestEqual("Event context 1", FGenericPlatformSentryConverters::StringMapToUnreal(testContext)[TEXT("ContextKey1")], TestContext[TEXT("ContextKey1")]);
				TestEqual("Event context 2", FGenericPlatformSentryConverters::StringMapToUnreal(testContext)[TEXT("ContextKey2")], TestContext[TEXT("ContextKey2")]);
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
#endif
}

#endif
