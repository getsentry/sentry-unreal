// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryTests.h"
#include "SentryBreadcrumb.h"

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryBreadcrumbSpec, "Sentry.SentryBreadcrumb", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryBreadcrumb* SentryBreadcrumb;
END_DEFINE_SPEC(SentryBreadcrumbSpec)

void SentryBreadcrumbSpec::Define()
{
	BeforeEach([this]()
	{
		SentryBreadcrumb = NewObject<USentryBreadcrumb>();
	});

	Describe("Breadcrumb params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestMessage = FString(TEXT("Test message"));
			const FString TestType = FString(TEXT("Test type"));
			const FString TestCategory = FString(TEXT("Test category"));

			TMap<FString, FString> TestData;
			TestData.Add(TEXT("Key1"), TEXT("Val1"));
			TestData.Add(TEXT("Key2"), TEXT("Val2"));

			SentryBreadcrumb->SetLevel(ESentryLevel::Fatal);
			SentryBreadcrumb->SetMessage(TestMessage);
			SentryBreadcrumb->SetType(TestType);
			SentryBreadcrumb->SetCategory(TestCategory);
			SentryBreadcrumb->SetData(TestData);

			TestEqual("Breadcrumb level", SentryBreadcrumb->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Breadcrumb message", SentryBreadcrumb->GetMessage(), TestMessage);
			TestEqual("Breadcrumb type", SentryBreadcrumb->GetType(), TestType);
			TestEqual("Breadcrumb category", SentryBreadcrumb->GetCategory(), TestCategory);

			TMap<FString, FString> ReceivedData = SentryBreadcrumb->GetData();
			TestEqual("Data 1", ReceivedData[TEXT("Key1")], TestData[TEXT("Key1")]);
			TestEqual("Data 2", ReceivedData[TEXT("Key2")], TestData[TEXT("Key2")]);
		});
	});
}

#endif