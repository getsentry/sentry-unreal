// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryMetricTest.h"

#include "SentryPlayground/SentryPlayground.h"

#include "SentrySubsystem.h"
#include "SentryUnit.h"
#include "SentryVariant.h"

#include "HAL/PlatformProcess.h"

void FSentryMetricTest::Run()
{
	USentrySubsystem* Subsystem = GetSubsystem();

	FString TestId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);

	Subsystem->SetAttribute(TEXT("global_attr"), FSentryVariant(TEXT("global_value")));

	Subsystem->SetAttribute(TEXT("global_removed"), FSentryVariant(TEXT("should_not_appear")));
	Subsystem->RemoveAttribute(TEXT("global_removed"));

	TMap<FString, FSentryVariant> CounterAttributes;
	CounterAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	CounterAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	TMap<FString, FSentryVariant> DistributionAttributes;
	DistributionAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	DistributionAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	TMap<FString, FSentryVariant> GaugeAttributes;
	GaugeAttributes.Add(TEXT("test_id"), FSentryVariant(TestId));
	GaugeAttributes.Add(TEXT("to_be_removed"), FSentryVariant(TEXT("original_value")));

	Subsystem->AddCountWithAttributes(TEXT("test.integration.counter"), 1, CounterAttributes);
	Subsystem->AddDistributionWithAttributes(TEXT("test.integration.distribution"), 42.5f, FSentryUnit(ESentryUnit::Millisecond), DistributionAttributes);
	Subsystem->AddGaugeWithAttributes(TEXT("test.integration.gauge"), 15.0f, FSentryUnit(ESentryUnit::Byte), GaugeAttributes);

	UE_LOG(LogSentrySample, Display, TEXT("METRIC_TRIGGERED: %s\n"), *TestId);

	// Ensure events were flushed
	Subsystem->Close();

	FPlatformProcess::Sleep(1.0f);

	CompleteWithResult(true);
}
