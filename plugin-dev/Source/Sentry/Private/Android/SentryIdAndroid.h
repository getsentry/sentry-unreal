// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

#include "Interface/SentryIdInterface.h"

class SentryIdAndroid : public ISentryId
{
public:
	SentryIdAndroid();
	SentryIdAndroid(jobject id);
	virtual ~SentryIdAndroid() override;

	jobject GetNativeObject();

	virtual FString ToString() const override;

private:
	jobject IdAndroid;
};