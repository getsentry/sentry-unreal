// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "SentryTests.h"

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

	Describe("Event fingerprint", [this]()
	{
		It("should persist its value", [this]()
		{
			TArray<FString> InFingerprint = { TEXT("F1"), TEXT("F2"), TEXT("F3") };

			SentryEvent->SetFingerprint(InFingerprint);
			TArray<FString> OutFingerprint = SentryEvent->GetFingerprint();

			TestEqual("Fingerprint elements count", OutFingerprint.Num(), InFingerprint.Num());
			TestEqual("Fingerprint first element", OutFingerprint[0], InFingerprint[0]);
			TestEqual("Fingerprint second element", OutFingerprint[1], InFingerprint[1]);
			TestEqual("Fingerprint third element", OutFingerprint[2], InFingerprint[2]);
		});

		It("can be emptry", [this]()
		{
			TArray<FString> InFingerprint = {};

			SentryEvent->SetFingerprint(InFingerprint);
			TArray<FString> OutFingerprint = SentryEvent->GetFingerprint();

			TestEqual("Fingerprint elements count", OutFingerprint.Num(), InFingerprint.Num());
			TestEqual("Fingerprint is empty", OutFingerprint.Num(), 0);
		});
	});
}

#endif