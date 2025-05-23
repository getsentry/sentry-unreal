// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

class ISentryId;

class FNullSentryUserFeedback final : public ISentryUserFeedback
{
public:
	FNullSentryUserFeedback(TSharedPtr<ISentryId> eventId) {}

	virtual ~FNullSentryUserFeedback() override = default;

	virtual void SetName(const FString& name) override {}
	virtual FString GetName() const override { return TEXT(""); }
	virtual void SetEmail(const FString& email) override {}
	virtual FString GetEmail() const override { return TEXT(""); }
	virtual void SetComment(const FString& comment) override {}
	virtual FString GetComment() const override { return TEXT(""); }
};

typedef FNullSentryUserFeedback FPlatformSentryUserFeedback;
