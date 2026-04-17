// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryInitOnlyTest : public FSentryBaseIntegrationTest
{
public:
	FSentryInitOnlyTest() : FSentryBaseIntegrationTest(TEXT("init-only")) {}

	virtual void Run() override;
};
