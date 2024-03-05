// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryMessageAndroid : public FSentryJavaObjectWrapper
{
public:
	SentryMessageAndroid(const FString& message);
	SentryMessageAndroid(jobject message);

	void SetupClassMethods();

	virtual FString ToString() const;

private:
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
};