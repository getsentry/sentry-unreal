// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

#include "Interface/SentryEventInterface.h"

class SentryEventAndroid : public ISentryEvent, public FJavaClassObject
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);

	static FName GetClassName();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	FJavaClassMethod SetMessageMethod;
	FJavaClassMethod GetMessageMethod;
	FJavaClassMethod SetLevelMethod;
	FJavaClassMethod GetLevelMethod;
};