// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryId;

class ISentryEvent
{
public:
	virtual ~ISentryEvent() = default;

	virtual TSharedPtr<ISentryId> GetId() const = 0;
	virtual void SetMessage(const FString& message) = 0;
	virtual FString GetMessage() const = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual ESentryLevel GetLevel() const = 0;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) = 0;
	virtual TArray<FString> GetFingerprint() = 0;
	virtual void SetTagValue(const FString& key, const FString& value) = 0;
	virtual FString GetTagValue(const FString& key) const = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetTags(const TMap<FString, FString>& tags) = 0;
	virtual TMap<FString, FString> GetTags() const = 0;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& Values) = 0;
	virtual TMap<FString, FString> GetContext(const FString& key) const = 0;
	virtual void RemoveContext(const FString& key) = 0;
	virtual void SetExtraValue(const FString& key, const FString& value) = 0;
	virtual FString GetExtraValue(const FString& key) const = 0;
	virtual void RemoveExtra(const FString& key) = 0;
	virtual void SetExtras(const TMap<FString, FString>& extras) = 0;
	virtual TMap<FString, FString> GetExtras() const = 0;
	virtual bool IsCrash() const = 0;
	virtual bool IsAnr() const = 0;
};
