// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryMessage : public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryMessage(const FString& message);
	FAndroidSentryMessage(jobject message);

	void SetupClassMethods();

	virtual FString ToString() const;

private:
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
};

typedef FAndroidSentryMessage FPlatformSentryMessage;