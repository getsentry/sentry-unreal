// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "Interface/SentryEventInterface.h"

@class SentryEvent;

class SentryEventIOS : public ISentryEvent
{
public:
	SentryEventIOS();
	SentryEventIOS(SentryEvent* event);
	virtual ~SentryEventIOS() override;

	SentryEvent* GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual void SetLevel(ESentryLevel level) override;

private:
	SentryEvent* EventIOS;
};
