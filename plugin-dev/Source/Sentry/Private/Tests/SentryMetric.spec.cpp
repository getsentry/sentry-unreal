// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryMetric.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryMetric.h"

#if WITH_AUTOMATION_TESTS && (PLATFORM_ANDROID || USE_SENTRY_NATIVE)

BEGIN_DEFINE_SPEC(SentryMetricSpec, "Sentry.SentryMetric", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryMetric* SentryMetric;
END_DEFINE_SPEC(SentryMetricSpec)

void SentryMetricSpec::Define()
{
	BeforeEach([this]()
	{
		SentryMetric = USentryMetric::Create(CreateSharedSentryMetric());
	});

	Describe("Metric params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestName = TEXT("request.duration");
			const ESentryMetricType TestType = ESentryMetricType::Gauge;
			const float TestValue = 42.5f;
			const FSentryUnit TestUnit = FSentryUnit(ESentryUnit::Millisecond);

			SentryMetric->SetName(TestName);
			SentryMetric->SetType(TestType);
			SentryMetric->SetValue(TestValue);
			SentryMetric->SetUnit(TestUnit);

			TestEqual("Metric name", SentryMetric->GetName(), TestName);
			TestEqual("Metric type", SentryMetric->GetType(), TestType);
			TestEqual("Metric value", SentryMetric->GetValue(), TestValue);
			TestEqual("Metric unit", SentryMetric->GetUnit().ToString(), TestUnit.ToString());
		});

		It("should handle all metric types", [this]()
		{
			SentryMetric->SetType(ESentryMetricType::Counter);
			TestEqual("Counter type", SentryMetric->GetType(), ESentryMetricType::Counter);

			SentryMetric->SetType(ESentryMetricType::Gauge);
			TestEqual("Gauge type", SentryMetric->GetType(), ESentryMetricType::Gauge);

			SentryMetric->SetType(ESentryMetricType::Distribution);
			TestEqual("Distribution type", SentryMetric->GetType(), ESentryMetricType::Distribution);
		});

		It("should reject Unknown type", [this]()
		{
			SentryMetric->SetType(ESentryMetricType::Counter);
			SentryMetric->SetType(ESentryMetricType::Unknown);

			TestEqual("Type should remain Counter", SentryMetric->GetType(), ESentryMetricType::Counter);
		});

		It("should handle empty name", [this]()
		{
			const FString EmptyName = TEXT("");

			SentryMetric->SetName(EmptyName);

			TestEqual("Empty name", SentryMetric->GetName(), EmptyName);
		});

		It("should handle predefined unit round-trip", [this]()
		{
			SentryMetric->SetUnit(FSentryUnit(ESentryUnit::Nanosecond));
			TestEqual("Nanosecond unit", SentryMetric->GetUnit().ToString(), TEXT("nanosecond"));

			SentryMetric->SetUnit(FSentryUnit(ESentryUnit::Megabyte));
			TestEqual("Megabyte unit", SentryMetric->GetUnit().ToString(), TEXT("megabyte"));

			SentryMetric->SetUnit(FSentryUnit(ESentryUnit::Percent));
			TestEqual("Percent unit", SentryMetric->GetUnit().ToString(), TEXT("percent"));
		});

		It("should handle custom unit string", [this]()
		{
			const FString CustomUnit = TEXT("requests");

			SentryMetric->SetUnit(FSentryUnit(CustomUnit));

			TestEqual("Custom unit", SentryMetric->GetUnit().ToString(), CustomUnit);
		});

		It("should handle None unit as empty string", [this]()
		{
			SentryMetric->SetUnit(FSentryUnit(ESentryUnit::None));

			TestEqual("None unit", SentryMetric->GetUnit().ToString(), TEXT(""));
		});
	});
}

#endif
