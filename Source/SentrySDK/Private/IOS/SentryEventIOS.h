// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

@class SentryEvent;

class SentryEventIOS
{
public:
	SentryEventIOS();
	SentryEventIOS(SentryEvent* event);

	SentryEvent* GetNativeObject();

	void SetMessage(const FString& message);
	void SetLevel(ESentryLevel level);

private:
	SentryEvent* EventIOS;
};
