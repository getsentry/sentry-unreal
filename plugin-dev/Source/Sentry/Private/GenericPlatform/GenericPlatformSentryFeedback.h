// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryFeedbackInterface.h"

#if USE_SENTRY_NATIVE

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

private:
	sentry_value_t Feedback;
};

typedef FGenericPlatformSentryFeedback FPlatformSentryFeedback;

#endif
