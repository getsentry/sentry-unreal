// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppBeforeMetricHandler.h"

#include "SentryMetric.h"

USentryMetric* UCppBeforeMetricHandler::HandleBeforeMetric_Implementation(USentryMetric* Metric)
{
	Metric->SetAttribute(TEXT("handler_added"), FSentryVariant(TEXT("added_value")));
	Metric->RemoveAttribute(TEXT("to_be_removed"));

	return Super::HandleBeforeMetric_Implementation(Metric);
}
