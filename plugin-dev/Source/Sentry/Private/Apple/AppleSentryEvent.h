// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

@class SentryEvent;

class FAppleSentryEvent : public ISentryEvent
{
public:
	FAppleSentryEvent();
	FAppleSentryEvent(SentryEvent* event);
	virtual ~FAppleSentryEvent() override;

	SentryEvent* GetNativeObject();

	virtual TSharedPtr<ISentryId> GetId() const override;
	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	SentryEvent* EventApple;
};

typedef FAppleSentryEvent FPlatformSentryEvent;
