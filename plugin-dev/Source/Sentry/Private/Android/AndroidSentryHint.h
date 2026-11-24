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
	virtual int32 RemoveAttachments(const FString& filenamePattern) override;
	virtual void ClearAttachments() override;

private:
	FSentryJavaMethod AddAttachmentMethod;
	FSentryJavaMethod GetAttachmentsMethod;
	FSentryJavaMethod ReplaceAttachmentsMethod;
	FSentryJavaMethod ClearAttachmentsMethod;
};

typedef FAndroidSentryHint FPlatformSentryHint;
