// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

#include "Interface/SentryIdInterface.h"

class SentryIdAndroid : public ISentryId, public FJavaClassObject
{
public:
	SentryIdAndroid();
	SentryIdAndroid(jobject id);

	static FName GetClassName();

	virtual FString ToString() const override;

private:
	FJavaClassMethod ToStringMethod;
};