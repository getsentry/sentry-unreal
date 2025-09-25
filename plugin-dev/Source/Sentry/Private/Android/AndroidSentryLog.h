// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryLog : public ISentryLog, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryLog();
	FAndroidSentryLog(jobject logEvent);
	FAndroidSentryLog(const FString& message, ESentryLevel level, const FString& category);

	void SetupClassMethods();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
};

typedef FAndroidSentryLog FPlatformSentryLog;