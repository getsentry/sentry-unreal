// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

@class SentryEvent;

class SentryEventApple : public ISentryEvent
{
public:
	SentryEventApple();
	SentryEventApple(SentryEvent* event);
	virtual ~SentryEventApple() override;

	SentryEvent* GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	SentryEvent* EventApple;
};
