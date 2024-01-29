// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryEvent.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"
#include "SentrySpan.h"

#include "UObject/UObjectGlobals.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentrySubsystemSpec, "Sentry.SentrySubsystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	USentrySubsystem* SentrySubsystem;
END_DEFINE_SPEC(SentrySubsystemSpec)

void SentrySubsystemSpec::Define()
{
	BeforeEach([this]()
	{
		SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

		if(SentrySubsystem && !SentrySubsystem->IsEnabled())
		{
			SentrySubsystem->Initialize();
		}
	});

	Describe("Capture Message", [this]()
	{
		It("should return a non-null Event ID if message captured", [this]()
		{
			const USentryId* eventId = SentrySubsystem->CaptureMessage(FString(TEXT("Automation: Sentry test message")), ESentryLevel::Debug);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return non-null Event ID if scoped version used", [this]()
		{
			const FConfigureScopeDelegate testDelegate;
			const USentryId* eventId = SentrySubsystem->CaptureMessageWithScope(FString(TEXT("Automation: Sentry test message with scope")), testDelegate, ESentryLevel::Debug);
			TestNotNull("Event ID is non-null", eventId);
		});
	});

	Describe("Capture Event", [this]()
	{
		It("should return a non-null Event ID if event captured", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const USentryId* eventId = SentrySubsystem->CaptureEvent(testEvent);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return non-null Event ID if scoped version used", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const FConfigureScopeDelegate testDelegate;

			const USentryId* eventId = SentrySubsystem->CaptureEventWithScope(testEvent, testDelegate);
			TestNotNull("Event ID is non-null", eventId);
		});
	});

	Describe("Transaction", [this]()
	{
		It("should be started and finished", [this]()
		{
			USentryTransaction* transaction = SentrySubsystem->StartTransaction(TEXT("Automation transaction"), TEXT("Automation operation"));
			TestNotNull("Transaction is non-null", transaction);
			TestFalse("Transaction is not finished", transaction->IsFinished());

			USentrySpan* span = transaction->StartChild(TEXT("Automation span"), TEXT("Description text"));
			TestNotNull("Span is non-null", span);
			TestFalse("Span is not finished", span->IsFinished());

			span->Finish();
			TestTrue("Span is finished", span->IsFinished());

			transaction->Finish();
			TestTrue("Transaction is finished", transaction->IsFinished());
		});

		It("should be started and finished with specific context", [this]()
		{
			USentryTransactionContext* transactionContext = NewObject<USentryTransactionContext>();
			transactionContext->Initialize(TEXT("Automation transaction"), TEXT("Automation operation"));

			USentryTransaction* transaction = SentrySubsystem->StartTransactionWithContext(transactionContext);
			TestNotNull("Transaction is non-null", transaction);
			TestFalse("Transaction is not finished", transaction->IsFinished());

			transaction->Finish();
			TestTrue("Transaction is finished", transaction->IsFinished());
		});
	});

	AfterEach([this]
	{
		SentrySubsystem->Close();
	});
}

#endif