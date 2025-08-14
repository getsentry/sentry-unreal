// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryFeedbackInterface.h"

class ISentryId;

class FNullSentryFeedback final : public ISentryFeedback
{
public:
	FNullSentryFeedback(const FString& message) {}

	virtual ~FNullSentryFeedback() override = default;

	virtual FString GetMessage() const override { return TEXT(""); }
	virtual void SetName(const FString& name) override {}
	virtual FString GetName() const override { return TEXT(""); }
	virtual void SetContactEmail(const FString& email) override {}
	virtual FString GetContactEmail() const override { return TEXT(""); }
	virtual void SetAssociatedEvent(const FString& eventId) override {}
	virtual FString GetAssociatedEvent() const override { return TEXT(""); }
};

typedef FNullSentryFeedback FPlatformSentryFeedback;
