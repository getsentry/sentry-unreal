// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryLogTest : public FSentryBaseIntegrationTest
{
public:
	FSentryLogTest() : FSentryBaseIntegrationTest(TEXT("log-capture")) {}

	virtual void Run() override;
};
