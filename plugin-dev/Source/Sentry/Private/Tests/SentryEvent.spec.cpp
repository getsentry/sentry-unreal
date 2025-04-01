// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryTests.h"
#include "SentryEvent.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryEvent.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryEventSpec, "Sentry.SentryEvent", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryEvent* SentryEvent;
END_DEFINE_SPEC(SentryEventSpec)

void SentryEventSpec::Define()
{
	BeforeEach([this]()
	{
		SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());
	});

	Describe("Event params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestMessage = FString(TEXT("Test event message"));

			SentryEvent->SetLevel(ESentryLevel::Fatal);
			SentryEvent->SetMessage(TestMessage);

			TestEqual("Event level", SentryEvent->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Event message", SentryEvent->GetMessage(), TestMessage);
			TestFalse("Event ID is non-empty", SentryEvent->GetId().IsEmpty());
		});
	});
}

#endif