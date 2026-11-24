// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryTracingTest : public FSentryBaseIntegrationTest
{
public:
	FSentryTracingTest() : FSentryBaseIntegrationTest(TEXT("tracing-capture")) {}

	virtual void Run() override;
};
