// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryEventInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryEvent : public ISentryEvent
{
public:
	FGenericPlatformSentryEvent();
	FGenericPlatformSentryEvent(sentry_value_t event, bool isCrash = false);
	virtual ~FGenericPlatformSentryEvent() override;

	sentry_value_t GetNativeObject();

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
	sentry_value_t Event;

	bool IsCrashEvent;
};

typedef FGenericPlatformSentryEvent FPlatformSentryEvent;

#endif
