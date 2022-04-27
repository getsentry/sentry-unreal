// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

#include "Interface/SentryEventInterface.h"

class SentryEventAndroid : public ISentryEvent
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);
	virtual ~SentryEventAndroid() override;

	jobject GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	jobject EventAndroid;
};
