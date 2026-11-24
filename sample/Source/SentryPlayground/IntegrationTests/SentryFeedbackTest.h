// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

class FSentryFeedbackTest : public FSentryBaseIntegrationTest
{
public:
	FSentryFeedbackTest()
		: FSentryBaseIntegrationTest(TEXT("feedback-capture")) {}

	virtual void Run() override;
};
