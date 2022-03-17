// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Android/AndroidJNI.h"

#include "SentryDataTypes.h"
#include "Interface/SentryEventInterface.h"

class SentryEventAndroid : public ISentryEvent
{
public:
	SentryEventAndroid();
	SentryEventAndroid(jobject event);
	virtual ~SentryEventAndroid() override;

	jobject GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual void SetLevel(ESentryLevel level) override;

private:
	jobject EventAndroid;
};
