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
	virtual void SetTag(const FString& key, const FString& value) override {}
	virtual FString GetTag(const FString& key) const override { return TEXT(""); }
	virtual bool TryGetTag(const FString& key, FString& value) const override { return false; }
	virtual void RemoveTag(const FString& key) override {}
	virtual void SetTags(const TMap<FString, FString>& tags) override {}
	virtual TMap<FString, FString> GetTags() const override { return TMap<FString, FString>(); }
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override {};
	virtual TMap<FString, FSentryVariant> GetContext(const FString& key) const override { return TMap<FString, FSentryVariant>(); }
	virtual bool TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const override { return false; }
	virtual void RemoveContext(const FString& key) override {}
	virtual void SetExtra(const FString& key, const FSentryVariant& value) override {}
	virtual FSentryVariant GetExtra(const FString& key) const override { return FSentryVariant(); }
	virtual bool TryGetExtra(const FString& key, FSentryVariant& value) const override { return false; }
	virtual void RemoveExtra(const FString& key) override {}
	virtual void SetExtras(const TMap<FString, FSentryVariant>& extras) override {}
	virtual TMap<FString, FSentryVariant> GetExtras() const override { return TMap<FString, FSentryVariant>(); }
	virtual bool IsCrash() const override { return false; }
	virtual bool IsAnr() const override { return false; }
};

typedef FNullSentryEvent FPlatformSentryEvent;
