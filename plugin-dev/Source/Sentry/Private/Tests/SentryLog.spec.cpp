// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLog.h"
#include "SentryTests.h"

#include "HAL/Platform.h"
#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryLog.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryLogSpec, "Sentry.SentryLog", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryLog* SentryLog;
END_DEFINE_SPEC(SentryLogSpec)

void SentryLogSpec::Define()
{
	BeforeEach([this]()
	{
		SentryLog = USentryLog::Create(CreateSharedSentryLog());
	});

	Describe("Log params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestBody = FString(TEXT("Test log body"));
			const ESentryLevel TestLevel = ESentryLevel::Warning;

			SentryLog->SetBody(TestBody);
			SentryLog->SetLevel(TestLevel);

			TestEqual("Log body", SentryLog->GetBody(), TestBody);
			TestEqual("Log level", SentryLog->GetLevel(), TestLevel);
		});

		It("should handle different log levels", [this]()
		{
			const FString TestBody = FString(TEXT("Test message"));

			// Test Debug level
			SentryLog->SetBody(TestBody);
			SentryLog->SetLevel(ESentryLevel::Debug);
			TestEqual("Debug level", SentryLog->GetLevel(), ESentryLevel::Debug);

			// Test Info level
			SentryLog->SetLevel(ESentryLevel::Info);
			TestEqual("Info level", SentryLog->GetLevel(), ESentryLevel::Info);

			// Test Warning level
			SentryLog->SetLevel(ESentryLevel::Warning);
			TestEqual("Warning level", SentryLog->GetLevel(), ESentryLevel::Warning);

			// Test Error level
			SentryLog->SetLevel(ESentryLevel::Error);
			TestEqual("Error level", SentryLog->GetLevel(), ESentryLevel::Error);

			// Test Fatal level
			SentryLog->SetLevel(ESentryLevel::Fatal);
			TestEqual("Fatal level", SentryLog->GetLevel(), ESentryLevel::Fatal);

			// Test invalid level falls back to Debug
#if PLATFORM_APPLE
			AddExpectedError(TEXT("Unknown Sentry level value used"), EAutomationExpectedErrorFlags::Contains, 0);
#endif
			SentryLog->SetLevel(static_cast<ESentryLevel>(18));
			TestEqual("Invalid level - default Debug", SentryLog->GetLevel(), ESentryLevel::Debug);
		});

		It("should handle empty body", [this]()
		{
			const FString EmptyBody = FString(TEXT(""));

			SentryLog->SetBody(EmptyBody);
			SentryLog->SetLevel(ESentryLevel::Info);

			TestEqual("Empty body", SentryLog->GetBody(), EmptyBody);
			TestEqual("Level with empty body", SentryLog->GetLevel(), ESentryLevel::Info);
		});
	});

	Describe("Log attributes", [this]()
	{
		It("can be set and retrieved as strings", [this]()
		{
			const FString AttributeKey = TEXT("user_name");
			const FSentryVariant AttributeValue = FSentryVariant(TEXT("John Doe"));

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant RetrievedValue = SentryLog->GetAttribute(AttributeKey);
			TestEqual("String attribute value", RetrievedValue.GetValue<FString>(), TEXT("John Doe"));
		});

		It("can be set and retrieved as integers", [this]()
		{
			const FString AttributeKey = TEXT("user_id");
			const FSentryVariant AttributeValue = FSentryVariant(12345);

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant RetrievedValue = SentryLog->GetAttribute(AttributeKey);
			TestEqual("Integer attribute value", RetrievedValue.GetValue<int>(), 12345);
		});

		It("can be set and retrieved as booleans", [this]()
		{
			const FString AttributeKey = TEXT("is_premium");
			const FSentryVariant AttributeValue = FSentryVariant(true);

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant RetrievedValue = SentryLog->GetAttribute(AttributeKey);
			TestEqual("Boolean attribute value", RetrievedValue.GetValue<bool>(), true);
		});

		It("can be set and retrieved as floats", [this]()
		{
			const FString AttributeKey = TEXT("score");
			const FSentryVariant AttributeValue = FSentryVariant(99.5f);

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant RetrievedValue = SentryLog->GetAttribute(AttributeKey);
			TestEqual("Float attribute value", RetrievedValue.GetValue<float>(), 99.5f);
		});

		It("support TryGetAttribute for successful retrieval", [this]()
		{
			const FString AttributeKey = TEXT("test_key");
			const FSentryVariant AttributeValue = FSentryVariant(TEXT("test_value"));

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant RetrievedValue;
			bool Success = SentryLog->TryGetAttribute(AttributeKey, RetrievedValue);

			TestTrue("TryGetAttribute should succeed", Success);
			TestEqual("TryGetAttribute value", RetrievedValue.GetValue<FString>(), TEXT("test_value"));
		});

		It("return false from TryGetAttribute for non-existent keys", [this]()
		{
			FSentryVariant RetrievedValue;
			bool Success = SentryLog->TryGetAttribute(TEXT("non_existent_key"), RetrievedValue);

			TestFalse("TryGetAttribute should fail for non-existent key", Success);
		});

		It("can be removed", [this]()
		{
			const FString AttributeKey = TEXT("temp_key");
			const FSentryVariant AttributeValue = FSentryVariant(TEXT("temp_value"));

			SentryLog->SetAttribute(AttributeKey, AttributeValue);

			FSentryVariant ValueBeforeRemoval;
			TestTrue("Attribute should exist before removal", SentryLog->TryGetAttribute(AttributeKey, ValueBeforeRemoval));

			SentryLog->RemoveAttribute(AttributeKey);

			FSentryVariant ValueAfterRemoval;
			TestFalse("Attribute should not exist after removal", SentryLog->TryGetAttribute(AttributeKey, ValueAfterRemoval));
		});

		It("can be added in bulk", [this]()
		{
			TMap<FString, FSentryVariant> Attributes;
			Attributes.Add(TEXT("attr1"), FSentryVariant(TEXT("value1")));
			Attributes.Add(TEXT("attr2"), FSentryVariant(42));
			Attributes.Add(TEXT("attr3"), FSentryVariant(true));

			SentryLog->AddAttributes(Attributes);

			FSentryVariant Attr1 = SentryLog->GetAttribute(TEXT("attr1"));
			FSentryVariant Attr2 = SentryLog->GetAttribute(TEXT("attr2"));
			FSentryVariant Attr3 = SentryLog->GetAttribute(TEXT("attr3"));

			TestEqual("First attribute", Attr1.GetValue<FString>(), TEXT("value1"));
			TestEqual("Second attribute", Attr2.GetValue<int>(), 42);
			TestEqual("Third attribute", Attr3.GetValue<bool>(), true);
		});

		It("can be overwritten", [this]()
		{
			const FString AttributeKey = TEXT("counter");

			SentryLog->SetAttribute(AttributeKey, FSentryVariant(10));
			TestEqual("Initial value", SentryLog->GetAttribute(AttributeKey).GetValue<int>(), 10);

			SentryLog->SetAttribute(AttributeKey, FSentryVariant(20));
			TestEqual("Updated value", SentryLog->GetAttribute(AttributeKey).GetValue<int>(), 20);
		});

		It("are preserved when adding new ones", [this]()
		{
			SentryLog->SetAttribute(TEXT("existing_attr"), FSentryVariant(TEXT("existing_value")));

			TMap<FString, FSentryVariant> NewAttributes;
			NewAttributes.Add(TEXT("new_attr"), FSentryVariant(TEXT("new_value")));

			SentryLog->AddAttributes(NewAttributes);

			FSentryVariant ExistingValue = SentryLog->GetAttribute(TEXT("existing_attr"));
			FSentryVariant NewValue = SentryLog->GetAttribute(TEXT("new_attr"));

			TestEqual("Existing attribute preserved", ExistingValue.GetValue<FString>(), TEXT("existing_value"));
			TestEqual("New attribute added", NewValue.GetValue<FString>(), TEXT("new_value"));
		});
	});
}

#endif