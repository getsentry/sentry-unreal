// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryBreadcrumb;
class ISentryAttachment;

class ISentryScope
{
public:
	virtual ~ISentryScope() = default;

	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) = 0;
	virtual void ClearBreadcrumbs() = 0;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) = 0;
	virtual void ClearAttachments() = 0;
	virtual void SetTagValue(const FString& key, const FString& value) = 0;
	virtual FString GetTagValue(const FString& key) const = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetTags(const TMap<FString, FString>& tags) = 0;
	virtual TMap<FString, FString> GetTags() const = 0;	
	virtual void SetDist(const FString& dist) = 0;
	virtual FString GetDist() const = 0;
	virtual void SetEnvironment(const FString& environment) = 0;
	virtual FString GetEnvironment() const = 0;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) = 0;
	virtual TArray<FString> GetFingerprint() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) = 0;
	virtual void RemoveContext(const FString& key) = 0;
	virtual void SetExtraValue(const FString& key, const FString& value) = 0;
	virtual FString GetExtraValue(const FString& key) const = 0;
	virtual void RemoveExtra(const FString& key) = 0;
	virtual void SetExtras(const TMap<FString, FString>& extras) = 0;
	virtual TMap<FString, FString> GetExtras() const = 0;
	virtual void Clear() = 0;
};