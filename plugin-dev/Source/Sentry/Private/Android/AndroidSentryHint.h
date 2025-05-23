// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryHintInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryHint : public ISentryHint, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryHint();
	FAndroidSentryHint(jobject hint);

	void SetupClassMethods();

	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;

private:
	FSentryJavaMethod AddAttachmentMethod;
};

typedef FAndroidSentryHint FPlatformSentryHint;
