// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeMetricHandler.h"
#include "SentryMetric.h"

USentryMetric* USentryBeforeMetricHandler::HandleBeforeMetric_Implementation(USentryMetric* Metric)
{
	return Metric;
}
