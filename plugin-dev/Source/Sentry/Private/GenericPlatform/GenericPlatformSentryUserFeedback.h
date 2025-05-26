// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryUserFeedbackInterface.h"

#if USE_SENTRY_NATIVE

class ISentryId;

class FGenericPlatformSentryUserFeedback : public ISentryUserFeedback
{
public:
	FGenericPlatformSentryUserFeedback();
	FGenericPlatformSentryUserFeedback(TSharedPtr<ISentryId> eventId);
	virtual ~FGenericPlatformSentryUserFeedback() override;

	sentry_value_t GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetComment(const FString& comment) override;
	virtual FString GetComment() const override;

private:
	sentry_value_t UserFeedback;
};

typedef FGenericPlatformSentryUserFeedback FPlatformSentryUserFeedback;

#endif
