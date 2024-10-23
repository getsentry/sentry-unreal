// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class ISentryId;

class SentryUserFeedbackAndroid : public ISentryUserFeedback, public FSentryJavaObjectWrapper
{
public:
	SentryUserFeedbackAndroid(TSharedPtr<ISentryId> eventId);

	void SetupClassMethods();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod SetEmailMethod;
	FSentryJavaMethod GetEmailMethod;
	FSentryJavaMethod SetCommentMethod;
	FSentryJavaMethod GetCommentMethod;
};