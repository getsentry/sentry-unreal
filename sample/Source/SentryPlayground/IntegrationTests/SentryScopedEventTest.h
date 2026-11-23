// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryScopedEventTest : public FSentryBaseIntegrationTest
{
public:
	FSentryScopedEventTest() : FSentryBaseIntegrationTest(TEXT("scoped-event-capture")) {}

	virtual void Run() override;
};
