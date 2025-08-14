// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryFeedback.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryFeedback.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryFeedbackSpec, "Sentry.SentryFeedback", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryFeedback* SentryFeedback;
END_DEFINE_SPEC(SentryFeedbackSpec)

void SentryFeedbackSpec::Define()
{
	BeforeEach([this]()
	{
		SentryFeedback = USentryFeedback::Create(MakeShareable(new FPlatformSentryFeedback(TEXT("Test feedback"))));
	});

	Describe("Feedback message", [this]()
	{
		It("should not be empty", [this]()
		{
			TestFalse("Message", SentryFeedback->GetMessage().IsEmpty());
		});
	});

	Describe("Feedback name", [this]()
	{
		It("should be empty if not set", [this]()
		{
			TestTrue("Name", SentryFeedback->GetName().IsEmpty());
		});

		It("should be empty if initialized with empty string", [this]()
		{
			SentryFeedback->SetName(TEXT(""));
			TestTrue("Name", SentryFeedback->GetName().IsEmpty());
		});

		It("should retain its value", [this]()
		{
			const FString TestName = TEXT("John Doe");
			SentryFeedback->SetName(TestName);
			TestEqual("Name", SentryFeedback->GetName(), TestName);
		});
	});

	Describe("Feedback contact email", [this]()
	{
		It("should be empty if not set", [this]()
		{
			TestTrue("Contact email", SentryFeedback->GetContactEmail().IsEmpty());
		});

		It("should be empty if initialized with empty string", [this]()
		{
			SentryFeedback->SetContactEmail(TEXT(""));
			TestTrue("Contact email", SentryFeedback->GetContactEmail().IsEmpty());
		});

		It("should retain its value", [this]()
		{
			const FString TestEmail = TEXT("feedback-mail@example.org");
			SentryFeedback->SetContactEmail(TestEmail);
			TestEqual("Contact email", SentryFeedback->GetContactEmail(), TestEmail);
		});
	});

	Describe("Feedback associated event", [this]()
	{
		It("should be empty if not set", [this]()
		{
			TestTrue("Event Id", SentryFeedback->GetAssociatedEvent().IsEmpty());
		});

		It("should be empty if initialized with empty string", [this]()
		{
			SentryFeedback->SetAssociatedEvent(TEXT(""));
			TestTrue("Event Id", SentryFeedback->GetAssociatedEvent().IsEmpty());
		});

		It("should retain its value", [this]()
		{
			const FString TestEventId = TEXT("c3829f10764848442d813c4124cf44a0");
			SentryFeedback->SetAssociatedEvent(TestEventId);
			TestEqual("Event Id", SentryFeedback->GetAssociatedEvent(), TestEventId);
		});
	});
}

#endif
