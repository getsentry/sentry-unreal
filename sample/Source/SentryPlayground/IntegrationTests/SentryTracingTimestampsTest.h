// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryTracingTimestampsTest : public FSentryBaseIntegrationTest
{
public:
	FSentryTracingTimestampsTest() : FSentryBaseIntegrationTest(TEXT("tracing-timestamps-capture")) {}

	virtual void Run() override;
};
