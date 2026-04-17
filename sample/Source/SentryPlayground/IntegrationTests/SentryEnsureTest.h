// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryEnsureTest : public FSentryBaseIntegrationTest
{
public:
	FSentryEnsureTest() : FSentryBaseIntegrationTest(TEXT("ensure-capture")) {}

	virtual void Run() override;
};
