// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryVariant.h"

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
	virtual void SetTag(const FString& key, const FString& value) = 0;
	virtual FString GetTag(const FString& key) const = 0;
	virtual bool TryGetTag(const FString& key, FString& value) const = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetTags(const TMap<FString, FString>& tags) = 0;
	virtual TMap<FString, FString> GetTags() const = 0;
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) = 0;
	virtual TMap<FString, FSentryVariant> GetContext(const FString& key) const = 0;
	virtual bool TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const = 0;
	virtual void RemoveContext(const FString& key) = 0;
	virtual void SetExtra(const FString& key, const FSentryVariant& value) = 0;
	virtual FSentryVariant GetExtra(const FString& key) const = 0;
	virtual bool TryGetExtra(const FString& key, FSentryVariant& value) const = 0;
	virtual void RemoveExtra(const FString& key) = 0;
	virtual void SetExtras(const TMap<FString, FSentryVariant>& extras) = 0;
	virtual TMap<FString, FSentryVariant> GetExtras() const = 0;
	virtual bool IsCrash() const = 0;
	virtual bool IsAnr() const = 0;
};
