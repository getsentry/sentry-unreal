// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryFeedbackInterface.h"

@class SentryFeedback;

class FAppleSentryFeedback : public ISentryFeedback
{
public:
	FAppleSentryFeedback(const FString& message);
	virtual ~FAppleSentryFeedback() override;

	virtual FString GetMessage() const override;
	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetContactEmail(const FString& email) override;
	virtual FString GetContactEmail() const override;
	virtual void SetAssociatedEvent(const FString& eventId) override;
	virtual FString GetAssociatedEvent() const override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;

	static SentryFeedback* CreateSentryFeedback(TSharedPtr<FAppleSentryFeedback> feedback);

private:
	FString Message;
	FString Name;
	FString Email;
	FString EventId;

	TArray<TSharedPtr<ISentryAttachment>> Attachments;
};

typedef FAppleSentryFeedback FPlatformSentryFeedback;