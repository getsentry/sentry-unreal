// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryLog : public ISentryLog, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryLog();
	FAndroidSentryLog(jobject logEvent);
	FAndroidSentryLog(const FString& body, ESentryLevel level);

	void SetupClassMethods();

	virtual void SetBody(const FString& body) override;
	virtual FString GetBody() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	FSentryJavaMethod SetBodyMethod;
	FSentryJavaMethod GetBodyMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
};

typedef FAndroidSentryLog FPlatformSentryLog;