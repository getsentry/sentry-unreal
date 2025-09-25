// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

@class SentryLog;

class FAppleSentryLog : public ISentryLog
{
public:
	FAppleSentryLog();
	FAppleSentryLog(SentryLog* log);
	FAppleSentryLog(const FString& message, ESentryLevel level, const FString& category);
	virtual ~FAppleSentryLog() override;

	SentryLog* GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	SentryLog* LogApple;
};

typedef FAppleSentryLog FPlatformSentryLog;