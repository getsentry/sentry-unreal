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
			const ESentryLogLevel TestLevel = ESentryLogLevel::Warning;

			SentryLog->SetBody(TestBody);
			SentryLog->SetLevel(TestLevel);

			TestEqual("Log body", SentryLog->GetBody(), TestBody);
			TestEqual("Log level", SentryLog->GetLevel(), TestLevel);
		});

		It("should handle different log levels", [this]()
		{
			const FString TestBody = FString(TEXT("Test message"));

			SentryLog->SetBody(TestBody);

			// Test Trace level
			SentryLog->SetLevel(ESentryLogLevel::Trace);
			TestEqual("Trace level", SentryLog->GetLevel(), ESentryLogLevel::Trace);

			// Test Debug level
			SentryLog->SetLevel(ESentryLogLevel::Debug);
			TestEqual("Debug level", SentryLog->GetLevel(), ESentryLogLevel::Debug);

			// Test Info level
			SentryLog->SetLevel(ESentryLogLevel::Info);
			TestEqual("Info level", SentryLog->GetLevel(), ESentryLogLevel::Info);

			// Test Warning level
			SentryLog->SetLevel(ESentryLogLevel::Warning);
			TestEqual("Warning level", SentryLog->GetLevel(), ESentryLogLevel::Warning);

			// Test Error level
			SentryLog->SetLevel(ESentryLogLevel::Error);
			TestEqual("Error level", SentryLog->GetLevel(), ESentryLogLevel::Error);

			// Test Fatal level
			SentryLog->SetLevel(ESentryLogLevel::Fatal);
			TestEqual("Fatal level", SentryLog->GetLevel(), ESentryLogLevel::Fatal);

			// Test invalid level falls back to Debug
#if PLATFORM_APPLE && !USE_SENTRY_NATIVE
			AddExpectedError(TEXT("Unknown Sentry level value used"), EAutomationExpectedErrorFlags::Contains, 0);
#endif
			SentryLog->SetLevel(static_cast<ESentryLogLevel>(18));
			TestEqual("Invalid level - default Debug", SentryLog->GetLevel(), ESentryLogLevel::Debug);
		});

		It("should change between any two log levels", [this]()
		{
			const UEnum* LevelEnum = StaticEnum<ESentryLogLevel>();
			const int32 LevelCount = LevelEnum->NumEnums() - 1;

			for (int32 FromIndex = 0; FromIndex < LevelCount; ++FromIndex)
			{
				for (int32 ToIndex = 0; ToIndex < LevelCount; ++ToIndex)
				{
					const ESentryLogLevel FromLevel = static_cast<ESentryLogLevel>(LevelEnum->GetValueByIndex(FromIndex));
					const ESentryLogLevel ToLevel = static_cast<ESentryLogLevel>(LevelEnum->GetValueByIndex(ToIndex));

					SentryLog->SetLevel(FromLevel);
					SentryLog->SetLevel(ToLevel);

					TestEqual(*FString::Printf(TEXT("%s to %s"), *LevelEnum->GetNameStringByIndex(FromIndex), *LevelEnum->GetNameStringByIndex(ToIndex)),
						SentryLog->GetLevel(), ToLevel);
				}
			}
		});

		It("should handle empty body", [this]()
		{
			const FString EmptyBody = FString(TEXT(""));

			SentryLog->SetBody(EmptyBody);
			SentryLog->SetLevel(ESentryLogLevel::Info);

			TestEqual("Empty body", SentryLog->GetBody(), EmptyBody);
			TestEqual("Level with empty body", SentryLog->GetLevel(), ESentryLogLevel::Info);
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