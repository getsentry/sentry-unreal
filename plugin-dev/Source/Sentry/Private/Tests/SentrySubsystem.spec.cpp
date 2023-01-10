// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "SentryModule.h"

#include "Interface/SentrySubsystemInterface.h"

#if PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentrySubsystemDesktop.h"
#elif PLATFORM_MAC
#include "Apple/SentrySubsystemApple.h"
#endif

#include "SentrySettings.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentrySubsystemSpec, "Sentry.SentrySubsystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
	TSharedPtr<ISentrySubsystem> SentrySubsystemImpl;
END_DEFINE_SPEC(SentrySubsystemSpec)

void SentrySubsystemSpec::Define()
{
	BeforeEach([this]()
	{
		// Subsystem implementation is used in order to avoid unnecessary complications with proper USentrySubsystem instantiation

#if PLATFORM_WINDOWS || PLATFORM_LINUX
		SentrySubsystemImpl = MakeShareable(new SentrySubsystemDesktop());
#elif PLATFORM_MAC
		SentrySubsystemImpl = MakeShareable(new SentrySubsystemApple());
#endif

		USentrySettings* Settings = FSentryModule::Get().GetSettings();
		SentrySubsystemImpl->InitWithSettings(Settings);
	});

	Describe("Capture Message", [this]()
	{
		It("should return a non-null Event ID if message captured", [this]()
		{
			const USentryId* eventId = SentrySubsystemImpl->CaptureMessage(FString(TEXT("Automation: Sentry test message")), ESentryLevel::Debug);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return null if scoped version used", [this]()
		{
			const FConfigureScopeDelegate testDelegate;
			const USentryId* eventId = SentrySubsystemImpl->CaptureMessageWithScope(FString(TEXT("Automation: Sentry test message with scope")), testDelegate, ESentryLevel::Debug);
			TestNull("Event ID is null", eventId);
		});
	});

	Describe("Capture Event", [this]()
	{
		It("should return a non-null Event ID if event captured", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const USentryId* eventId = SentrySubsystemImpl->CaptureEvent(testEvent);
			TestNotNull("Event ID is non-null", eventId);
		});

		It("should always return null if scoped version used", [this]()
		{
			USentryEvent* testEvent = NewObject<USentryEvent>();
			testEvent->SetMessage(TEXT("Automation: Sentry test event message"));

			const FConfigureScopeDelegate testDelegate;

			const USentryId* eventId = SentrySubsystemImpl->CaptureEventWithScope(testEvent, testDelegate);
			TestNull("Event ID is null", eventId);
		});
	});

	AfterEach([this]
	{
		SentrySubsystemImpl->Close();
	});
}

#endif