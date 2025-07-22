// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

class ISentryId;

@class SentryFeedback;

class FAppleSentryUserFeedback : public ISentryUserFeedback
{
public:
	FAppleSentryUserFeedback(TSharedPtr<ISentryId> eventId);
	virtual ~FAppleSentryUserFeedback() override;

	void SetNativeObject(SentryFeedback* feedback);
	SentryFeedback* GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

	static SentryFeedback* CreateSentryFeedback(TSharedPtr<FAppleSentryUserFeedback> feedback);

private:
	TSharedPtr<ISentryId> EventId;
	FString Name;
	FString Email;
	FString Comment;

	SentryFeedback* UserFeedbackApple;
};

typedef FAppleSentryUserFeedback FPlatformSentryUserFeedback;