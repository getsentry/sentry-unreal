// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

@class SentryScope;

class SentryScopeIOS
{
public:
	void InitWithNativeObject(SentryScope* scope);

	void SetTagValue(const FString& Key, const FString& Value);
	void RemoveTag(const FString& Key);
	void SetTags(const TMap<FString, FString>& Tags);
	void SetDist(const FString& Dist);
	void SetEnvironment(const FString& Environment);
	void SetFingerprint(const TArray<FString>& Fingerprint);
	void SetLevel(ESentryLevel Level);
	void Clear();

private:
	SentryScope* _scopeIOS;
};
