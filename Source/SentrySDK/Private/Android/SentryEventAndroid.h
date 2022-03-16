// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Android/AndroidJNI.h"

#include "SentryDataTypes.h"

class SentryEventAndroid
{
public:
	~SentryEventAndroid();

	void Init();
	void InitWithNativeObject(jobject event);
	jobject GetNativeObject();

	void SetMessage(const FString& message);
	void SetLevel(ESentryLevel level);

private:
	jobject EventAndroid;
};
