// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryScopeInterface.h"

class FNullSentryScope final : public ISentryScope
{
public:
	virtual ~FNullSentryScope() override = default;

	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override {}
	virtual void ClearBreadcrumbs() override {}
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual void ClearAttachments() override {}
	virtual void SetTag(const FString& key, const FString& value) override {}
	virtual FString GetTag(const FString& key) const override { return TEXT(""); }
	virtual bool TryGetTag(const FString& key, FString& value) const override { return false; }
	virtual void RemoveTag(const FString& key) override {}
	virtual void SetTags(const TMap<FString, FString>& tags) override {}
	virtual TMap<FString, FString> GetTags() const override { return {}; }
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override {}
	virtual TArray<FString> GetFingerprint() const override { return {}; }
	virtual void SetLevel(ESentryLevel level) override {}
	virtual ESentryLevel GetLevel() const override { return ESentryLevel::Debug; }
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override {}
	virtual void RemoveContext(const FString& key) override {}
	virtual void SetExtra(const FString& key, const FString& value) override {}
	virtual FString GetExtra(const FString& key) const override { return TEXT(""); }
	virtual bool TryGetExtra(const FString& key, FSentryVariant& value) const override { return false; }
	virtual void RemoveExtra(const FString& key) override {}
	virtual void SetExtras(const TMap<FString, FString>& extras) override {}
	virtual TMap<FString, FString> GetExtras() const override { return {}; }
	virtual void Clear() override {}
};

typedef FNullSentryScope FPlatformSentryScope;
