// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryTests.h"
#include "SentryTraceSamplingHandler.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"

#include "HAL/PlatformSentryTransactionContext.h"

#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

TDelegate<void(USentrySamplingContext*)> UTraceSamplingTestHandler::OnTraceSamplingTestHandler;

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryTraceSamplingSpec, "Sentry.SentryTraceSampling", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryTransactionContext* TransactionContext;
	FSentryTransactionOptions TransactionOptions;
END_DEFINE_SPEC(SentryTraceSamplingSpec)

void SentryTraceSamplingSpec::Define()
{
	BeforeEach([this]()
	{
		TransactionContext = USentryTransactionContext::Create(
			MakeShareable(new FPlatformSentryTransactionContext(TEXT("Test transaction"), TEXT("Test operation"))));

		TransactionOptions = FSentryTransactionOptions();
		TransactionOptions.CustomSamplingContext.Add(TEXT("test_key"), FSentryVariant(TEXT("test_value")));
		TransactionOptions.CustomSamplingContext.Add(TEXT("numeric_key"), FSentryVariant(42));
	});

	Describe("Trace sampling", [this]()
	{
		It("should execute callback and provide a valid sampling context", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

			SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
			{
				Settings->EnableTracing = true;
				Settings->SamplingType = ESentryTracesSamplingType::TracesSampler;
				Settings->TracesSampler = UTraceSamplingTestHandler::StaticClass();
			}));

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.BindLambda([this](USentrySamplingContext* SamplingContext)
			{
				TestNotNull("Sampling context should not be null", SamplingContext);
				if (SamplingContext)
				{
					USentryTransactionContext* CustomContext = SamplingContext->GetTransactionContext();
					TestNotNull("Transaction context should be available in sampling context", CustomContext);
					if (CustomContext)
					{
						TestEqual("Transaction name should match", CustomContext->GetName(), TEXT("Test transaction"));
						TestEqual("Transaction operation should match", CustomContext->GetOperation(), TEXT("Test operation"));
					}

					TMap<FString, FSentryVariant> CustomData = SamplingContext->GetCustomSamplingContext();
					TestTrue("Custom sampling context should contain test_key", CustomData.Contains(TEXT("test_key")));
					TestTrue("Custom sampling context should contain numeric_key", CustomData.Contains(TEXT("numeric_key")));

					if (CustomData.Contains(TEXT("test_key")))
					{
						TestEqual("test_key should have correct value", CustomData[TEXT("test_key")].GetValue<FString>(), TEXT("test_value"));
					}

					if (CustomData.Contains(TEXT("numeric_key")))
					{
						TestEqual("numeric_key should have correct value", CustomData[TEXT("numeric_key")].GetValue<int32>(), 42);
					}
				}
			});

			USentryTransaction* Transaction = SentrySubsystem->StartTransactionWithContextAndOptions(TransactionContext, TransactionOptions);

			if (Transaction)
			{
				Transaction->Finish();
			}

			UTraceSamplingTestHandler::OnTraceSamplingTestHandler.Unbind();

			SentrySubsystem->Close();
		});
	});
}

#endif