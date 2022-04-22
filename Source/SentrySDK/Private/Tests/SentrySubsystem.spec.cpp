// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "SentryModule.h"

#if PLATFORM_WINDOWS || PLATFORM_MAC
#include "Desktop/SentrySubsystemDesktop.h"
#endif

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentrySubsystemSpec, "Sentry.SentrySubsystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	TSharedPtr<ISentrySubsystem> SentrySubsystemDesktopImpl;
END_DEFINE_SPEC(SentrySubsystemSpec)

void SentrySubsystemSpec::Define()
{
	BeforeEach([this]()
	{
#if PLATFORM_WINDOWS || PLATFORM_MAC
		// Subsystem implementation for desktop is used in order to avoid unnecessary complications with proper USentrySubsystem instantiation 
		SentrySubsystemDesktopImpl = MakeShareable(new SentrySubsystemDesktop());
#endif

		const USentrySettings* Settings = FSentryModule::Get().GetSettings();
		SentrySubsystemDesktopImpl->InitWithSettings(Settings);
	});

	Describe("Capture Message", [this]()
	{
		It("should return a non-null Event ID if message captured", [this]()
		{
			const USentryId* eventId = SentrySubsystemDesktopImpl->CaptureMessage(FString(TEXT("Automation: Sentry test message")), ESentryLevel::Debug);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return null if scoped version used", [this]()
		{
			const FConfigureScopeDelegate testDelegate;
			const USentryId* eventId = SentrySubsystemDesktopImpl->CaptureMessageWithScope(FString(TEXT("Automation: Sentry test message with scope")), testDelegate, ESentryLevel::Debug);
			TestNull("Event ID is null", eventId);
		});
	});

	Describe("Capture Event", [this]()
	{
		It("should return a non-null Event ID if event captured", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const USentryId* eventId = SentrySubsystemDesktopImpl->CaptureEvent(testEvent);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return null if scoped version used", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const FConfigureScopeDelegate testDelegate;

			const USentryId* eventId = SentrySubsystemDesktopImpl->CaptureEventWithScope(testEvent, testDelegate);
			TestNull("Event ID is null", eventId);
		});
	});

	Describe("Set Level", [this]()
	{
		It("should affect events to be captured", [this]()
		{
			SentrySubsystemDesktopImpl->SetLevel(ESentryLevel::Fatal);

			USentryEvent* testEvent = NewObject<USentryEvent>();
			SentrySubsystemDesktopImpl->CaptureEvent(testEvent);

			TestEqual("Event level matches global settings", testEvent->GetLevel(), ESentryLevel::Fatal);
		});
	});

	AfterEach([this]
	{
		SentrySubsystemDesktopImpl->Close();
	});
}

#endif