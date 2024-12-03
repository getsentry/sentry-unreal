// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryTests.h"
#include "SentryScope.h"
#include "SentryEvent.h"

#include "SentryDataTypes.h"

#include "Interface/SentryScopeInterface.h"
#include "Interface/SentryEventInterface.h"

#include "Misc/AutomationTest.h"

#if PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryScopeDesktop.h"
#include "Desktop/SentryEventDesktop.h"
#include "Desktop/Infrastructure/SentryConvertorsDesktop.h"
#endif

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
		SentryScope = NewObject<USentryScope>();

		TestDist = TEXT("dist_str");
		TestEnvironment = TEXT("env_str");

		TestTags.Add(TEXT("TagsKey1"), TEXT("TagsVal1"));
		TestTags.Add(TEXT("TagsKey2"), TEXT("TagsVal2"));

		TestExtras.Add(TEXT("ExtrasKey1"), TEXT("ExtrasVal1"));
		TestExtras.Add(TEXT("ExtrasKey2"), TEXT("ExtrasVal2"));

		TestContext.Add(TEXT("ContextKey1"), TEXT("ContextVal1"));
		TestContext.Add(TEXT("ContextKey2"), TEXT("ContextVal2"));

		TestFingerprint = { TEXT("F1"), TEXT("F2"), TEXT("F3") };
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
			SentryScope->SetDist(TestDist);
			SentryScope->SetEnvironment(TestEnvironment);
			SentryScope->SetFingerprint(TestFingerprint);

			TestEqual("Scope level", SentryScope->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Scope dist", SentryScope->GetDist(), TestDist);
			TestEqual("Scope environment", SentryScope->GetEnvironment(), TestEnvironment);
			TestEqual("Scope fingerprint", SentryScope->GetFingerprint(), TestFingerprint);
		});
	});

	Describe("Scope params", [this]()
	{
		It("should be possible to clear", [this]()
		{
			SentryScope->SetDist(TestDist);
			SentryScope->SetEnvironment(TestEnvironment);
			SentryScope->SetFingerprint(TestFingerprint);
			SentryScope->SetTags(TestTags);
			SentryScope->SetExtras(TestExtras);

			SentryScope->Clear();

			TestTrue("Scope dist", SentryScope->GetDist().IsEmpty());
			TestTrue("Scope environment", SentryScope->GetEnvironment().IsEmpty());
			TestTrue("Scope fingerprint", SentryScope->GetFingerprint().Num() == 0);
			TestTrue("Scope tags", SentryScope->GetTags().Num() == 0);
			TestTrue("Scope extras", SentryScope->GetExtras().Num() == 0);
		});
	});

#if (PLATFORM_WINDOWS || PLATFORM_LINUX) && USE_SENTRY_NATIVE
	Describe("Scope params", [this]()
	{
		It("should be applied to event", [this]()
		{
			SentryScope->SetLevel(ESentryLevel::Fatal);
			SentryScope->SetDist(TestDist);
			SentryScope->SetEnvironment(TestEnvironment);
			SentryScope->SetFingerprint(TestFingerprint);
			SentryScope->SetTags(TestTags);
			SentryScope->SetExtras(TestExtras);
			SentryScope->SetContext(TEXT("TestContext"), TestContext);

			USentryEvent* SentryEvent = NewObject<USentryEvent>();

			TSharedPtr<SentryEventDesktop> EventDesktop = StaticCastSharedPtr<SentryEventDesktop>(SentryEvent->GetNativeImpl());

			StaticCastSharedPtr<SentryScopeDesktop>(SentryScope->GetNativeImpl())->Apply(EventDesktop);

			sentry_value_t NativeEvent = EventDesktop->GetNativeObject();

			sentry_value_t level = sentry_value_get_by_key(NativeEvent, "level");
			sentry_value_t dist = sentry_value_get_by_key(NativeEvent, "dist");
			sentry_value_t environment = sentry_value_get_by_key(NativeEvent, "environment");
			sentry_value_t fingerprint = sentry_value_get_by_key(NativeEvent, "fingerprint");
			sentry_value_t tags = sentry_value_get_by_key(NativeEvent, "tags");
			sentry_value_t extra = sentry_value_get_by_key(NativeEvent, "extra");

			sentry_value_t contexts = sentry_value_get_by_key(NativeEvent, "contexts");
			sentry_value_t testContext = sentry_value_get_by_key(contexts, "TestContext");

			TestEqual("Event level", SentryConvertorsDesktop::SentryLevelToUnreal(level), ESentryLevel::Fatal);
			TestEqual("Event dist", FString(sentry_value_as_string(dist)), TestDist);
			TestEqual("Event environment", FString(sentry_value_as_string(environment)), TestEnvironment);
			TestEqual("Event fingerprint", SentryConvertorsDesktop::StringArrayToUnreal(fingerprint), TestFingerprint);
			TestEqual("Event tags 1", SentryConvertorsDesktop::StringMapToUnreal(tags)[TEXT("TagsKey1")], TestTags[TEXT("TagsKey1")]);
			TestEqual("Event tags 2", SentryConvertorsDesktop::StringMapToUnreal(tags)[TEXT("TagsKey2")], TestTags[TEXT("TagsKey2")]);
			TestEqual("Event extra 1", SentryConvertorsDesktop::StringMapToUnreal(extra)[TEXT("ExtrasKey1")], TestExtras[TEXT("ExtrasKey1")]);
			TestEqual("Event extra 2", SentryConvertorsDesktop::StringMapToUnreal(extra)[TEXT("ExtrasKey2")], TestExtras[TEXT("ExtrasKey2")]);
			TestEqual("Event context 1", SentryConvertorsDesktop::StringMapToUnreal(testContext)[TEXT("ContextKey1")], TestContext[TEXT("ContextKey1")]);
			TestEqual("Event context 2", SentryConvertorsDesktop::StringMapToUnreal(testContext)[TEXT("ContextKey2")], TestContext[TEXT("ContextKey2")]);
		});
	});
#endif
}

#endif