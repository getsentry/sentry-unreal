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
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override {}
	virtual TMap<FString, FSentryVariant> GetContext(const FString& key) const override { return TMap<FString, FSentryVariant>(); }
	virtual bool TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const override { return false; }
	virtual void RemoveContext(const FString& key) override {}
	virtual void SetExtra(const FString& key, const FSentryVariant& value) override {}
	virtual FSentryVariant GetExtra(const FString& key) const override { return FSentryVariant(); }
	virtual bool TryGetExtra(const FString& key, FSentryVariant& value) const override { return false; }
	virtual void RemoveExtra(const FString& key) override {}
	virtual void SetExtras(const TMap<FString, FSentryVariant>& extras) override {}
	virtual TMap<FString, FSentryVariant> GetExtras() const override { return TMap<FString, FSentryVariant>(); }
	virtual void Clear() override {}
};

typedef FNullSentryScope FPlatformSentryScope;
