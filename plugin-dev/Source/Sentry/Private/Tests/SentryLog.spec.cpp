// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryLog.h"
#include "SentryTests.h"

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

			// Test Info level
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
}

#endif