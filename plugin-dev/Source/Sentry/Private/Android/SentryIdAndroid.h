// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryIdInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryIdAndroid : public ISentryId, public FSentryJavaObjectWrapper
{
public:
	SentryIdAndroid();
	SentryIdAndroid(jobject id);

	void SetupClassMethods();

	static FSentryJavaClass GetClassName();

	virtual FString ToString() const override;

private:
	FSentryJavaMethod ToStringMethod;
};