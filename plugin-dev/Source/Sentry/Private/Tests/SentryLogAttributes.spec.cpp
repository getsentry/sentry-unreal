// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryLog.h"
#include "SentrySettings.h"
#include "SentryTestBeforeLogHandler.h"
#include "SentryTests.h"
#include "SentryVariant.h"

#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryLogAttributesSpec, "Sentry.SentryLogAttributes", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentrySubsystem* SentrySubsystem;
END_DEFINE_SPEC(SentryLogAttributesSpec)

void SentryLogAttributesSpec::Define()
{
	BeforeEach([this]()
	{
		SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

		if (SentrySubsystem && SentrySubsystem->IsEnabled())
		{
			SentrySubsystem->Close();
		}

		SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([](USentrySettings* Settings)
		{
			Settings->BeforeLogHandler = UTestBeforeLogHandler::StaticClass();
			Settings->EnableStructuredLogging = true;
		}));
	});

	Describe("Global log attributes", [this]()
	{
		It("should be attached to captured logs", [this]()
		{
			bool bHandlerCalled = false;
			const FString TestBody = TEXT("Test log with global attribute");
			const FString TestCategory = TEXT("TestCategory");
			const FString GlobalAttrKey = TEXT("global.test.attr");
			const FString GlobalAttrValue = TEXT("global_value");

			// Set global attribute before capturing log
			SentrySubsystem->SetAttribute(GlobalAttrKey, FSentryVariant(GlobalAttrValue));

			UTestBeforeLogHandler::OnTestBeforeLogHandler.BindLambda([this, &bHandlerCalled, GlobalAttrKey, GlobalAttrValue](USentryLog* LogData)
			{
				bHandlerCalled = true;

				FSentryVariant RetrievedValue;
				bool bFound = LogData->TryGetAttribute(GlobalAttrKey, RetrievedValue);

				TestTrue("Global attribute should be present on log", bFound);
				if (bFound)
				{
					TestEqual("Global attribute value should match", RetrievedValue.GetValue<FString>(), GlobalAttrValue);
				}
			});

			SentrySubsystem->LogWarning(TestBody, TestCategory);

			TestTrue("BeforeLogHandler should be called", bHandlerCalled);

			UTestBeforeLogHandler::OnTestBeforeLogHandler.Unbind();
		});

		It("should support different value types", [this]()
		{
			bool bHandlerCalled = false;
			const FString TestBody = TEXT("Test log with typed attributes");
			const FString TestCategory = TEXT("TestCategory");

			// Set global attributes of different types
			SentrySubsystem->SetAttribute(TEXT("attr.string"), FSentryVariant(TEXT("string_value")));
			SentrySubsystem->SetAttribute(TEXT("attr.integer"), FSentryVariant(42));
			SentrySubsystem->SetAttribute(TEXT("attr.float"), FSentryVariant(3.14f));
			SentrySubsystem->SetAttribute(TEXT("attr.bool"), FSentryVariant(true));

			UTestBeforeLogHandler::OnTestBeforeLogHandler.BindLambda([this, &bHandlerCalled](USentryLog* LogData)
			{
				bHandlerCalled = true;

				FSentryVariant StringValue;
				FSentryVariant IntValue;
				FSentryVariant FloatValue;
				FSentryVariant BoolValue;

				if (LogData->TryGetAttribute(TEXT("attr.string"), StringValue))
				{
					TestEqual("String attribute", StringValue.GetValue<FString>(), TEXT("string_value"));
				}

				if (LogData->TryGetAttribute(TEXT("attr.integer"), IntValue))
				{
					TestEqual("Integer attribute", IntValue.GetValue<int32>(), 42);
				}

				if (LogData->TryGetAttribute(TEXT("attr.float"), FloatValue))
				{
					TestEqual("Float attribute", FloatValue.GetValue<float>(), 3.14f);
				}

				if (LogData->TryGetAttribute(TEXT("attr.bool"), BoolValue))
				{
					TestEqual("Bool attribute", BoolValue.GetValue<bool>(), true);
				}
			});

			SentrySubsystem->LogWarning(TestBody, TestCategory);

			TestTrue("BeforeLogHandler should be called", bHandlerCalled);

			UTestBeforeLogHandler::OnTestBeforeLogHandler.Unbind();
		});

		It("should not include removed attributes", [this]()
		{
			bool bHandlerCalled = false;
			const FString TestBody = TEXT("Test log after attribute removal");
			const FString TestCategory = TEXT("TestCategory");
			const FString AttrKey = TEXT("removable.attr");

			// Set and then remove the attribute
			SentrySubsystem->SetAttribute(AttrKey, FSentryVariant(TEXT("to_be_removed")));
			SentrySubsystem->RemoveAttribute(AttrKey);

			UTestBeforeLogHandler::OnTestBeforeLogHandler.BindLambda([this, &bHandlerCalled, AttrKey](USentryLog* LogData)
			{
				bHandlerCalled = true;

				FSentryVariant RetrievedValue;
				bool bFound = LogData->TryGetAttribute(AttrKey, RetrievedValue);

				TestFalse("Removed attribute should not be present on log", bFound);
			});

			SentrySubsystem->LogWarning(TestBody, TestCategory);

			TestTrue("BeforeLogHandler should be called", bHandlerCalled);

			UTestBeforeLogHandler::OnTestBeforeLogHandler.Unbind();
		});
	});

	AfterEach([this]()
	{
		SentrySubsystem->Close();
	});
}

#endif
