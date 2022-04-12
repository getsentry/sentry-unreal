// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

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
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	SentryEvent* EventIOS;
};
