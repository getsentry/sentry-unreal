// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryEvent.h"
#include "SentrySpan.h"
#include "SentryTests.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"

#include "HAL/PlatformSentryEvent.h"
#include "HAL/PlatformSentryTransactionContext.h"

#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "Misc/DateTime.h"
#include "UObject/UObjectGlobals.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentrySubsystemSpec, "Sentry.SentrySubsystem", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentrySubsystem* SentrySubsystem;
END_DEFINE_SPEC(SentrySubsystemSpec)

void SentrySubsystemSpec::Define()
{
	BeforeEach([this]()
	{
		SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

		if (SentrySubsystem && !SentrySubsystem->IsEnabled())
		{
			SentrySubsystem->Initialize();
		}
	});

	Describe("Capture Message", [this]()
	{
		It("should return a non-null Event ID if message captured", [this]()
		{
			FString eventId = SentrySubsystem->CaptureMessage(FString(TEXT("Automation: Sentry test message")), ESentryLevel::Debug);
			TestFalse("Event ID is non-empty", eventId.IsEmpty());
		});

		It("should always return non-null Event ID if scoped version used", [this]()
		{
			const FConfigureScopeNativeDelegate testDelegate;
			FString eventId = SentrySubsystem->CaptureMessageWithScope(FString(TEXT("Automation: Sentry test message with scope")), testDelegate, ESentryLevel::Debug);
			TestFalse("Event ID is non-empty", eventId.IsEmpty());
		});
	});

	Describe("Capture Event", [this]()
	{
		It("should return a non-null Event ID if event captured", [this]()
		{
			USentryEvent* testEvent = USentryEvent::Create(CreateSharedSentryEvent());
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			FString eventId = SentrySubsystem->CaptureEvent(testEvent);
			TestFalse("Event ID is non-empty", eventId.IsEmpty());
		});

		It("should always return non-null Event ID if scoped version used", [this]()
		{
			USentryEvent* testEvent = USentryEvent::Create(CreateSharedSentryEvent());
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const FConfigureScopeNativeDelegate testDelegate;

			FString eventId = SentrySubsystem->CaptureEventWithScope(testEvent, testDelegate);
			TestFalse("Event ID is non-empty", eventId.IsEmpty());
		});
	});

	Describe("Transaction", [this]()
	{
		It("should be started and finished", [this]()
		{
			USentryTransaction* transaction = SentrySubsystem->StartTransaction(TEXT("Automation transaction"), TEXT("Automation operation"));
			TestNotNull("Transaction is non-null", transaction);
			TestFalse("Transaction is not finished", transaction->IsFinished());

			USentrySpan* span = transaction->StartChildSpan(TEXT("Automation span"), TEXT("Description text"));
			TestNotNull("Span is non-null", span);
			TestFalse("Span is not finished", span->IsFinished());

			USentrySpan* childSpan = span->StartChildWithTimestamp(TEXT("Automation child span"), TEXT("Description text"), FDateTime::UtcNow().ToUnixTimestamp());
			TestNotNull("Child span is non-null", childSpan);
			TestFalse("Child span is not finished", childSpan->IsFinished());

			childSpan->FinishWithTimestamp(FDateTime::UtcNow().ToUnixTimestamp());
			TestTrue("Child span is finished", childSpan->IsFinished());

			span->Finish();
			TestTrue("Span is finished", span->IsFinished());

			transaction->Finish();
			TestTrue("Transaction is finished", transaction->IsFinished());
		});

		It("should be started and finished with specific context", [this]()
		{
			USentryTransactionContext* transactionContext =
				USentryTransactionContext::Create(
					CreateSharedSentryTransactionContext(TEXT("Automation transaction"), TEXT("Automation operation")));

			USentryTransaction* transaction = SentrySubsystem->StartTransactionWithContext(transactionContext);
			TestNotNull("Transaction is non-null", transaction);
			TestFalse("Transaction is not finished", transaction->IsFinished());

			transaction->Finish();
			TestTrue("Transaction is finished", transaction->IsFinished());
		});

		It("should be started and finished with specific context and timings", [this]()
		{
			USentryTransactionContext* transactionContext =
				USentryTransactionContext::Create(
					CreateSharedSentryTransactionContext(TEXT("Automation transaction"), TEXT("Automation operation")));

			USentryTransaction* transaction = SentrySubsystem->StartTransactionWithContextAndTimestamp(transactionContext, FDateTime::UtcNow().ToUnixTimestamp());
			TestNotNull("Transaction is non-null", transaction);
			TestFalse("Transaction is not finished", transaction->IsFinished());

			transaction->FinishWithTimestamp(FDateTime::UtcNow().ToUnixTimestamp());
			TestTrue("Transaction is finished", transaction->IsFinished());
		});
	});

	Describe("Transaction context", [this]()
	{
		It("should be created from trace and used to start a transaction", [this]()
		{
			const FString inTrace = FString(TEXT("2674eb52d5874b13b560236d6c79ce8a-a0f9fdf04f1a63df-1"));

			USentryTransactionContext* transactionContext = SentrySubsystem->ContinueTrace(inTrace, TArray<FString>());
			TestNotNull("Transaction context non-null", transactionContext);

			USentryTransaction* transaction = SentrySubsystem->StartTransactionWithContext(transactionContext);
			TestNotNull("Transaction is non-null", transaction);

			FString outTraceKey;
			FString outTraceValue;
			transaction->GetTrace(outTraceKey, outTraceValue);

			TArray<FString> outTraceParts;
			outTraceValue.ParseIntoArray(outTraceParts, TEXT("-"));
			TestTrue("Trace has valid amount of parts", outTraceParts.Num() >= 2);

			TestEqual("Trace header", outTraceKey, TEXT("sentry-trace"));
			TestEqual("Trace ID", outTraceParts[0], TEXT("2674eb52d5874b13b560236d6c79ce8a"));
		});
	});

	AfterEach([this]
	{
		SentrySubsystem->Close();
	});
}

#endif
