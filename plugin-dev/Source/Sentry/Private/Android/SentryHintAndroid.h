// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryHintInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryHintAndroid : public ISentryHint, public FSentryJavaObjectWrapper
{
public:
	SentryHintAndroid();
	SentryHintAndroid(jobject hint);

	void SetupClassMethods();

	virtual void AddAttachment(USentryAttachment* attachment) override;

private:
	FSentryJavaMethod AddAttachmentMethod;
};
