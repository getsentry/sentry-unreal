// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"
#include "SentryTraceSamplingHandler.h"
#include "SentryTests.h"

#include "HAL/PlatformSentryTransactionContext.h"

#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

TDelegate<void(USentrySamplingContext*)> UTraceSamplingTestHandler::OnTraceSamplingTestHandler;

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryTraceSamplingSpec, "Sentry.SentryTraceSampling", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentrySubsystem* SentrySubsystem;
	USentryTransactionContext* TransactionContext;
	FSentryTransactionOptions TransactionOptions;
END_DEFINE_SPEC(SentryTraceSamplingSpec)

void SentryTraceSamplingSpec::Define()
{
	BeforeEach([this]()
	{
		SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

		SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
		{
			Settings->SamplingType = ESentryTracesSamplingType::TracesSampler;
			Settings->TracesSampler = UTraceSamplingTestHandler::StaticClass();
		}));

		TransactionContext = USentryTransactionContext::Create(
			MakeShareable(new FPlatformSentryTransactionContext(TEXT("Test transaction"), TEXT("Test operation"))));

		TransactionOptions = FSentryTransactionOptions();
	});

	Describe("Trace sampler", [this]()
	{
		It("should be called and provide sampling context", [this]()
		{
			bool CallbackExecuted = false;
			USentrySamplingContext* ReceivedContext = nullptr;

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.BindLambda([&](USentrySamplingContext* SamplingContext)
			{
				CallbackExecuted = true;
				ReceivedContext = SamplingContext;
			});

			USentryTransaction* Transaction = SentrySubsystem->StartTransactionWithContextAndOptions(TransactionContext, TransactionOptions);

			TestTrue("Sampling callback should be executed", CallbackExecuted);
			TestNotNull("Sampling context should not be null", ReceivedContext);

			if (Transaction)
			{
				Transaction->Finish();
			}

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.Unbind();

			SentrySubsystem->Close();
		});

		It("should provide transaction context in sampling context", [this]()
		{
			USentryTransactionContext* CapturedTransactionContext = nullptr;

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.BindLambda([&](USentrySamplingContext* SamplingContext)
			{
				if (SamplingContext)
				{
					CapturedTransactionContext = SamplingContext->GetTransactionContext();
				}
			});

			USentryTransaction* Transaction = SentrySubsystem->StartTransactionWithContextAndOptions(TransactionContext, TransactionOptions);

			TestNotNull("Transaction context should be available in sampling context", CapturedTransactionContext);
			if (CapturedTransactionContext)
			{
				TestEqual("Transaction name should match", CapturedTransactionContext->GetName(), TEXT("Test transaction"));
				TestEqual("Transaction operation should match", CapturedTransactionContext->GetOperation(), TEXT("Test operation"));
			}

			if (Transaction)
			{
				Transaction->Finish();
			}

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.Unbind();

			SentrySubsystem->Close();
		});

		It("should provide custom sampling context data in sampling context", [this]()
		{
			TMap<FString, FSentryVariant> CapturedCustomData;

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.BindLambda([&](USentrySamplingContext* SamplingContext)
			{
				if (SamplingContext)
				{
					CapturedCustomData = SamplingContext->GetCustomSamplingContext();
				}
			});

			TransactionOptions.CustomSamplingContext.Add(TEXT("test_key"), FSentryVariant(TEXT("test_value")));
			TransactionOptions.CustomSamplingContext.Add(TEXT("numeric_key"), FSentryVariant(42));

			USentryTransaction* Transaction = SentrySubsystem->StartTransactionWithContextAndOptions(TransactionContext, TransactionOptions);

			TestTrue("Custom sampling context should contain test_key", CapturedCustomData.Contains(TEXT("test_key")));
			TestTrue("Custom sampling context should contain numeric_key", CapturedCustomData.Contains(TEXT("numeric_key")));

			if (CapturedCustomData.Contains(TEXT("test_key")))
			{
				TestEqual("test_key should have correct value", CapturedCustomData[TEXT("test_key")].GetValue<FString>(), TEXT("test_value"));
			}

			if (CapturedCustomData.Contains(TEXT("numeric_key")))
			{
				TestEqual("numeric_key should have correct value", CapturedCustomData[TEXT("numeric_key")].GetValue<int32>(), 42);
			}

			if (Transaction)
			{
				Transaction->Finish();
			}

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.Unbind();

			SentrySubsystem->Close();
		});
	});

	AfterEach([this]
	{
		SentrySubsystem->Close();
	});
}

#endif