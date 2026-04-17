// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryHangTest : public FSentryBaseIntegrationTest
{
public:
	FSentryHangTest() : FSentryBaseIntegrationTest(TEXT("hang-capture")) {}

	virtual void Run() override;
};
