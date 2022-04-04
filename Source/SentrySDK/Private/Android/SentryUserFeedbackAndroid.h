// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

#include "Interface/SentryUserFeedbackInterface.h"

class USentryId;

class SentryUserFeedbackAndroid : public ISentryUserFeedback
{
public:
	SentryUserFeedbackAndroid(USentryId* eventId);
	virtual ~SentryUserFeedbackAndroid() override;

	jobject GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual void SetEmail(const FString& email) override;
	virtual void SetComment(const FString& comment) override;

private:
	jobject UserFeedbackAndroid;
};