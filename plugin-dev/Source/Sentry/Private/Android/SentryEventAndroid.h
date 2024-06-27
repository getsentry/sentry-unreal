// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryEventAndroid : public ISentryEvent, public FSentryJavaObjectWrapper
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);

	void SetupClassMethods();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
	FSentryJavaMethod IsCrashMethod;
};