// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryFeedbackInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryAttachment;

class FGenericPlatformSentryFeedback : public ISentryFeedback
{
public:
	FGenericPlatformSentryFeedback();
	FGenericPlatformSentryFeedback(const FString& message);
	virtual ~FGenericPlatformSentryFeedback() override;

	sentry_value_t GetNativeObject();

	virtual FString GetMessage() const override;
	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetContactEmail(const FString& email) override;
	virtual FString GetContactEmail() const override;
	virtual void SetAssociatedEvent(const FString& eventId) override;
	virtual FString GetAssociatedEvent() const override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;

	sentry_feedback_hint_t* GetHintNativeObject();

protected:
	virtual void AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment);
	virtual void AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment);

	sentry_feedback_hint_t* Hint;

private:
	sentry_value_t Feedback;
};

typedef FGenericPlatformSentryFeedback FPlatformSentryFeedback;

#endif
