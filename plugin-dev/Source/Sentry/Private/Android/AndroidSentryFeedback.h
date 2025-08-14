// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryFeedbackInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class ISentryId;

class FAndroidSentryFeedback : public ISentryFeedback, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryFeedback(const FString& message);

	void SetupClassMethods();

	virtual FString GetMessage() const override;
	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetContactEmail(const FString& email) override;
	virtual FString GetContactEmail() const override;
	virtual void SetAssociatedEvent(const FString& eventId) override;
	virtual FString GetAssociatedEvent() const override;

private:
	FSentryJavaMethod GetMessageMethod;
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod SetContactEmailMethod;
	FSentryJavaMethod GetContactEmailMethod;
	FSentryJavaMethod SetAssociatedEventMethod;
	FSentryJavaMethod GetAssociatedEventMethod;
};

typedef FAndroidSentryFeedback FPlatformSentryFeedback;