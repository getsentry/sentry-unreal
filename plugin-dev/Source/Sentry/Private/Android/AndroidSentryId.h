// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryId : public ISentryId, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryId();
	FAndroidSentryId(const FString& id);
	FAndroidSentryId(jobject id);

	void SetupClassMethods();

	virtual FString ToString() const override;

private:
	FSentryJavaMethod ToStringMethod;
};

typedef FAndroidSentryId FPlatformSentryId;
