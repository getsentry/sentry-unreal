// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class SentryIdAndroid : public ISentryId, public FSentryJavaClassWrapper
{
public:
	SentryIdAndroid();
	SentryIdAndroid(jobject id);

	void SetupClassMethods();

	static FName GetClassName();

	virtual FString ToString() const override;

private:
	FSentryJavaClassMethod ToStringMethod;
};