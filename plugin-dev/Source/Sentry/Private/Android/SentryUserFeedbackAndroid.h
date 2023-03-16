// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

#include "Interface/SentryUserFeedbackInterface.h"

class USentryId;

class SentryUserFeedbackAndroid : public ISentryUserFeedback, public FJavaClassObject
{
public:
	SentryUserFeedbackAndroid(USentryId* eventId);

	static FName GetClassName();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	FJavaClassMethod SetNameMethod;
	FJavaClassMethod GetNameMethod;
	FJavaClassMethod SetEmailMethod;
	FJavaClassMethod GetEmailMethod;
	FJavaClassMethod SetCommentMethod;
	FJavaClassMethod GetCommentMethod;
};