// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryMessageTest : public FSentryBaseIntegrationTest
{
public:
	FSentryMessageTest() : FSentryBaseIntegrationTest(TEXT("message-capture")) {}

	virtual void Run() override;
};
