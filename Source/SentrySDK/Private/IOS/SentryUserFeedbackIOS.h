// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

class USentryId;
@class SentryUserFeedback;

class SentryUserFeedbackIOS : public ISentryUserFeedback
{
public:
	SentryUserFeedbackIOS(USentryId* eventId);
	virtual ~SentryUserFeedbackIOS() override;

	SentryUserFeedback* GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual void SetEmail(const FString& email) override;
	virtual void SetComment(const FString& comment) override;

private:
	SentryUserFeedback* UserFeedbackIOS;
};