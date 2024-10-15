// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryUserFeedbackInterface.h"

#if USE_SENTRY_NATIVE

class ISentryId;

class SentryUserFeedbackDesktop : public ISentryUserFeedback
{
public:
	SentryUserFeedbackDesktop();
	SentryUserFeedbackDesktop(TSharedPtr<ISentryId> eventId);
	virtual ~SentryUserFeedbackDesktop() override;

	sentry_value_t GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	sentry_value_t UserFeedbackDesktop;
};

#endif
