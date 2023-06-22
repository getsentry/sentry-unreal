// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryEventSpec, "Sentry.SentryEvent", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	USentryEvent* SentryEvent;
END_DEFINE_SPEC(SentryEventSpec)

void SentryEventSpec::Define()
{
	BeforeEach([this]()
	{
		SentryEvent = NewObject<USentryEvent>();
	});

	Describe("Event params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestMessage = FString(TEXT("Test event message"));

			SentryEvent->SetLevel(ESentryLevel::Fatal);
			SentryEvent->SetMessage(TestMessage);

			TestNotEqual("Event level", SentryEvent->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Event message", SentryEvent->GetMessage(), TestMessage);
		});
	});
}

#endif