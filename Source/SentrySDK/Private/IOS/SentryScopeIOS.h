// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

@class SentryScope;

class SentryScopeIOS
{
public:
	SentryScopeIOS();
	SentryScopeIOS(SentryScope* scope);

	SentryScope* GetNativeObject();

	void SetTagValue(const FString& key, const FString& value);
	void RemoveTag(const FString& key);
	void SetTags(const TMap<FString, FString>& tags);
	void SetDist(const FString& dist);
	void SetEnvironment(const FString& environment);
	void SetFingerprint(const TArray<FString>& fingerprint);
	void SetLevel(ESentryLevel level);
	void Clear();

private:
	SentryScope* ScopeIOS;
};
