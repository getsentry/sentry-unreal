// Copyright (c) 2025 Sentry. All Rights Reserved.

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
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	SentryEvent* EventApple;
};

typedef FAppleSentryEvent FPlatformSentryEvent;
