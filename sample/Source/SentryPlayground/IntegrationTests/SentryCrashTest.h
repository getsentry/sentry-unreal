// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "SentryBaseIntegrationTest.h"

enum class ESentryAppTerminationType : uint8;

class FSentryCrashTest : public FSentryBaseIntegrationTest
{
public:
	FSentryCrashTest(const FString& InName, ESentryAppTerminationType InCrashType)
		: FSentryBaseIntegrationTest(InName), CrashType(InCrashType) {}

	virtual void Run() override;

private:
	ESentryAppTerminationType CrashType;
};
