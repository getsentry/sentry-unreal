// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class SentryEventAndroid : public ISentryEvent, public FSentryJavaClassWrapper
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);

	void SetupClassMethods();

	static FName GetClassName();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	FSentryJavaClassMethod SetMessageMethod;
	FSentryJavaClassMethod GetMessageMethod;
	FSentryJavaClassMethod SetLevelMethod;
	FSentryJavaClassMethod GetLevelMethod;
};