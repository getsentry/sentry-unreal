// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeMetricHandler.h"
#include "Engine/Engine.h"
#include "SentryMetric.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryTestBeforeMetricHandler.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryMetric.h"

TDelegate<void(USentryMetric*)> UTestBeforeMetricHandler::OnTestBeforeMetricHandler;

#if WITH_AUTOMATION_TESTS && (PLATFORM_ANDROID || USE_SENTRY_NATIVE)

BEGIN_DEFINE_SPEC(SentryBeforeMetricHandlerSpec, "Sentry.SentryBeforeMetricHandler", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
END_DEFINE_SPEC(SentryBeforeMetricHandlerSpec)

void SentryBeforeMetricHandlerSpec::Define()
{
	Describe("BeforeMetricHandler functionality", [this]()
	{
		It("should be called when metric is emitted", [this]()
		{
			USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();

			bool bHandlerCalled = false;
			const FString TestKey = TEXT("test.counter");
			const int32 TestValue = 5;

			SentrySubsystem->InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateLambda([=](USentrySettings* Settings)
			{
				Settings->BeforeMetricHandler = UTestBeforeMetricHandler::StaticClass();
			}));

			UTestBeforeMetricHandler::OnTestBeforeMetricHandler.BindLambda([this, &bHandlerCalled, TestKey](USentryMetric* MetricData)
			{
				bHandlerCalled = true;
				TestEqual("Handler received correct name", MetricData->GetName(), TestKey);
				TestEqual("Handler received correct type", MetricData->GetType(), ESentryMetricType::Counter);
			});

			SentrySubsystem->AddCount(TestKey, TestValue);

			TestTrue("BeforeMetricHandler should be called", bHandlerCalled);

			UTestBeforeMetricHandler::OnTestBeforeMetricHandler.Unbind();
			SentrySubsystem->Close();
		});
	});
}

#endif
