// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryEventInterface.h"

class FNullSentryEvent final : public ISentryEvent
{
public:
	virtual ~FNullSentryEvent() override = default;

	virtual TSharedPtr<ISentryId> GetId() const override { return nullptr; }
	virtual void SetMessage(const FString& message) override {}
	virtual FString GetMessage() const override { return TEXT(""); }
	virtual void SetLevel(ESentryLevel level) override {}
	virtual ESentryLevel GetLevel() const override { return ESentryLevel::Debug; }
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override {}
	virtual TArray<FString> GetFingerprint() override { return TArray<FString>(); }
	virtual bool IsCrash() const override { return false; }
	virtual bool IsAnr() const override { return false; }
};

typedef FNullSentryEvent FPlatformSentryEvent;
