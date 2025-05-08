// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "Containers/RingBuffer.h"
#include "HAL/CriticalSection.h"

#include "Interface/SentryScopeInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryBreadcrumb;
class FGenericPlatformSentryEvent;

class FGenericPlatformSentryScope : public ISentryScope
{
public:
	FGenericPlatformSentryScope();
	FGenericPlatformSentryScope(const FGenericPlatformSentryScope& Scope);
	virtual ~FGenericPlatformSentryScope() override;

	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void ClearBreadcrumbs() override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void ClearAttachments() override;
	virtual void SetTagValue(const FString& key, const FString& value) override;
	virtual FString GetTagValue(const FString& key) const override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual TMap<FString, FString> GetTags() const override;
	virtual void SetDist(const FString& dist) override;
	virtual FString GetDist() const override;
	virtual void SetEnvironment(const FString& environment) override;
	virtual FString GetEnvironment() const override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveContext(const FString& key) override;
	virtual void SetExtraValue(const FString& key, const FString& value) override;
	virtual FString GetExtraValue(const FString& key) const override;
	virtual void RemoveExtra(const FString& key) override;
	virtual void SetExtras(const TMap<FString, FString>& extras) override;
	virtual TMap<FString, FString> GetExtras() const override;
	virtual void Clear() override;

	void Apply(TSharedPtr<FGenericPlatformSentryEvent> event);

private:
	FString Dist;
	FString Environment;

	TArray<FString> Fingerprint;

	TMap<FString, FString> Tags;
	TMap<FString, FString> Extra;

	TMap<FString, TMap<FString, FString>> Contexts;

	TRingBuffer<TSharedPtr<FGenericPlatformSentryBreadcrumb>> Breadcrumbs;

	ESentryLevel Level;

	FCriticalSection CriticalSection;
};

typedef FGenericPlatformSentryScope FPlatformSentryScope;

#endif