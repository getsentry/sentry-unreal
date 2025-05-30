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
	virtual void SetTagValue(const FString& key, const FString& value) override {}
	virtual FString GetTagValue(const FString& key) const override { return TEXT(""); }
	virtual void RemoveTag(const FString& key) override {}
	virtual void SetTags(const TMap<FString, FString>& tags) override {}
	virtual TMap<FString, FString> GetTags() const override { return TMap<FString, FString>(); }
	virtual void SetContext(const FString& key, const TMap<FString, FString>& Values) override {}
	virtual TMap<FString, FString> GetContext(const FString& key) const override { return TMap<FString, FString>(); }
	virtual void RemoveContext(const FString& key) override {}
	virtual void SetExtraValue(const FString& key, const FString& value) override {}
	virtual FString GetExtraValue(const FString& key) const override { return TEXT(""); }
	virtual void RemoveExtra(const FString& key) override {}
	virtual void SetExtras(const TMap<FString, FString>& extras) override {}
	virtual TMap<FString, FString> GetExtras() const override { return TMap<FString, FString>(); }
	virtual bool IsCrash() const override { return false; }
	virtual bool IsAnr() const override { return false; }
};

typedef FNullSentryEvent FPlatformSentryEvent;
