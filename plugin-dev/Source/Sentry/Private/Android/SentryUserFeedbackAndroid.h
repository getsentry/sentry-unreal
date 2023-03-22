// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class USentryId;

class SentryUserFeedbackAndroid : public ISentryUserFeedback, public FSentryJavaClassWrapper
{
public:
	SentryUserFeedbackAndroid(USentryId* eventId);

	void SetupClassMethods();

	static FName GetClassName();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	FSentryJavaClassMethod SetNameMethod;
	FSentryJavaClassMethod GetNameMethod;
	FSentryJavaClassMethod SetEmailMethod;
	FSentryJavaClassMethod GetEmailMethod;
	FSentryJavaClassMethod SetCommentMethod;
	FSentryJavaClassMethod GetCommentMethod;
};