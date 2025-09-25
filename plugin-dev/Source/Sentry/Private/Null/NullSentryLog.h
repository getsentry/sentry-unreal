// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

class FNullSentryLog final : public ISentryLog
{
public:
	virtual ~FNullSentryLog() override = default;

	virtual void SetMessage(const FString& message) override {}
	virtual FString GetMessage() const override { return TEXT(""); }
	virtual void SetLevel(ESentryLevel level) override {}
	virtual ESentryLevel GetLevel() const override { return ESentryLevel::Info; }
};

typedef FNullSentryLog FPlatformSentryLog;