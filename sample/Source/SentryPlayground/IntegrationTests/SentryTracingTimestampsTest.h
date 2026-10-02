// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryTracingTimestampsTest : public FSentryBaseIntegrationTest
{
public:
	FSentryTracingTimestampsTest() : FSentryBaseIntegrationTest(TEXT("tracing-timestamp")) {}

	virtual void Run() override;
};
