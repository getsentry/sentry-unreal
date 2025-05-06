#pragma once

#include "Interface/SentryUserFeedbackInterface.h"

class FNullSentryUserFeedback final : public ISentryUserFeedback
{
public:
	virtual ~FNullSentryUserFeedback() override = default;

	virtual void SetName(const FString& name) override {}
	virtual FString GetName() const override { return TEXT(""); }
	virtual void SetEmail(const FString& email) override {}
	virtual FString GetEmail() const override { return TEXT(""); }
	virtual void SetComment(const FString& comment) override {}
	virtual FString GetComment() const override { return TEXT(""); }
};

typedef FNullSentryUserFeedback FPlatformSentryUserFeedback;
