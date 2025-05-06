// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

class ISentryId;

@class SentryFeedback;

class FAppleSentryUserFeedback : public ISentryUserFeedback
{
public:
	FAppleSentryUserFeedback(TSharedPtr<ISentryId> eventId);
	virtual ~FAppleSentryUserFeedback() override;

	SentryFeedback* GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	SentryFeedback* UserFeedbackApple;
};

typedef FAppleSentryUserFeedback FPlatformSentryUserFeedback;