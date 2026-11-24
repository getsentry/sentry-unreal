// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryMetricTest : public FSentryBaseIntegrationTest
{
public:
	FSentryMetricTest() : FSentryBaseIntegrationTest(TEXT("metric-capture")) {}

	virtual void Run() override;
};
