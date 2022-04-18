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
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	jobject UserFeedbackAndroid;
};