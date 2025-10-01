// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

@class SentryLog;

class FAppleSentryLog : public ISentryLog
{
public:
	FAppleSentryLog();
	FAppleSentryLog(SentryLog* log);
	FAppleSentryLog(const FString& body, ESentryLevel level);
	virtual ~FAppleSentryLog() override;

	SentryLog* GetNativeObject();

	virtual void SetBody(const FString& body) override;
	virtual FString GetBody() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	SentryLog* LogApple;
};

typedef FAppleSentryLog FPlatformSentryLog;