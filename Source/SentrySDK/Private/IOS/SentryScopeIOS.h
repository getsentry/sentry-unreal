// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryScopeInterface.h"

@class SentryScope;

class SentryScopeIOS : public ISentryScope
{
public:
	SentryScopeIOS();
	SentryScopeIOS(SentryScope* scope);
	virtual ~SentryScopeIOS() override;

	SentryScope* GetNativeObject();

	virtual void SetTagValue(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual void SetDist(const FString& dist) override;
	virtual void SetEnvironment(const FString& environment) override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual void Clear() override;

private:
	SentryScope* ScopeIOS;
};
