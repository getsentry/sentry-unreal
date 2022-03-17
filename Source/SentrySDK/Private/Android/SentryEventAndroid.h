// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Android/AndroidJNI.h"

#include "SentryDataTypes.h"

class SentryEventAndroid
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);
	~SentryEventAndroid();

	jobject GetNativeObject();

	void SetMessage(const FString& message);
	void SetLevel(ESentryLevel level);

private:
	jobject EventAndroid;
};
