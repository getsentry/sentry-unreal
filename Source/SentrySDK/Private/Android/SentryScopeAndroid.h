// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Android/AndroidJNI.h"

#include "SentryDataTypes.h"
#include "Interface/SentryScopeInterface.h"

class SentryScopeAndroid : public ISentryScope
{
public:
	SentryScopeAndroid();
	SentryScopeAndroid(jobject scope);
	virtual ~SentryScopeAndroid() override;

	jobject GetNativeObject();

	virtual void SetTagValue(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual void SetDist(const FString& dist) override;
	virtual void SetEnvironment(const FString& environment) override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual void Clear() override;

private:
	jobject ScopeAndroid;
};
