// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryScope
{
public:
	virtual ~ISentryScope() = default;

	virtual void SetTagValue(const FString& key, const FString& value) = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetTags(const TMap<FString, FString>& tags) = 0;
	virtual void SetDist(const FString& dist) = 0;
	virtual void SetEnvironment(const FString& environment) = 0;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual void Clear() = 0;
};