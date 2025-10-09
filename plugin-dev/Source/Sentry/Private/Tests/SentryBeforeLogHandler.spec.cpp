// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeLogHandler.h"
#include "Engine/Engine.h"
#include "SentryLog.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryTestBeforeLogHandler.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryLog.h"

TDelegate<void(USentryLog*)> UTestBeforeLogHandler::OnTestBeforeLogHandler;

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryBeforeLogHandlerSpec, "Sentry.SentryBeforeLogHandler", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryBeforeLogHandler* BeforeLogHandler;
	USentryLog* TestLog;
END_DEFINE_SPEC(SentryBeforeLogHandlerSpec)

void SentryBeforeLogHandlerSpec::Define()
{
	BeforeEach([this]()
	{
		BeforeLogHandler = NewObject<USentryBeforeLogHandler>();
		TestLog = USentryLog::Create(CreateSharedSentryLog());
	});

	Describe("BeforeLogHandler functionality", [this]()
	{
		It("should return log data unchanged by default", [this]()
		{
			const FString TestBody = TEXT("Test log message");
			const ESentryLevel TestLevel = ESentryLevel::Info;

			TestLog->SetBody(TestBody);
			TestLog->SetLevel(TestLevel);

			USentryLog* Result = BeforeLogHandler->HandleBeforeLog(TestLog);

			TestEqual("Returned log should be same instance", Result, TestLog);
			TestEqual("Log body should be unchanged", Result->GetBody(), TestBody);
			TestEqual("Log level should be unchanged", Result->GetLevel(), TestLevel);
		});

		It("should be called when log is processed", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

			bool bHandlerCalled = false;
			const FString TestBody = TEXT("Test log for handler");
			const FString TestCategory = TEXT("LogTestCategory");
			const ESentryLevel TestLevel = ESentryLevel::Warning;

			SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
			{
				Settings->BeforeLogHandler = UTestBeforeLogHandler::StaticClass();
				Settings->EnableStructuredLogging = true; // Enable structured logging for the test
			}));

			UTestBeforeLogHandler::OnTestBeforeLogHandler.BindLambda([this, &bHandlerCalled, TestBody, TestCategory, TestLevel](USentryLog* LogData)
			{
				bHandlerCalled = true;
				const FString ExpectedBody = FString::Printf(TEXT("[%s] %s"), *TestCategory, *TestBody);
				TestEqual("Handler received correct body", LogData->GetBody(), ExpectedBody);
				TestEqual("Handler received correct level", LogData->GetLevel(), TestLevel);
			});

			SentrySubsystem->LogWarning(TestBody, TestCategory);

			TestTrue("BeforeLogHandler should be called", bHandlerCalled);

			UTestBeforeLogHandler::OnTestBeforeLogHandler.Unbind();
			SentrySubsystem->Close();
		});

		It("should handle null log data gracefully", [this]()
		{
			USentryLog* Result = BeforeLogHandler->HandleBeforeLog(nullptr);
			TestEqual("Should return nullptr for nullptr input", Result, static_cast<USentryLog*>(nullptr));
		});
	});

	Describe("Different log levels", [this]()
	{
		It("should handle all log levels correctly", [this]()
		{
			const FString TestBody = TEXT("Test message");
			TArray<ESentryLevel> TestLevels = {
				ESentryLevel::Debug,
				ESentryLevel::Info,
				ESentryLevel::Warning,
				ESentryLevel::Error,
				ESentryLevel::Fatal
			};

			for (ESentryLevel Level : TestLevels)
			{
				TestLog->SetBody(TestBody);
				TestLog->SetLevel(Level);

				USentryLog* Result = BeforeLogHandler->HandleBeforeLog(TestLog);

				TestEqual(FString::Printf(TEXT("Level %d should be preserved"), (int32)Level),
					Result->GetLevel(), Level);
			}
		});
	});
}

#endif