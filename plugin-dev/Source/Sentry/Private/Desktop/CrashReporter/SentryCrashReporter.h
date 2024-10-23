// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_NATIVE

class SentryUserDesktop;

class FJsonObject;

class SentryCrashReporter
{
public:
	SentryCrashReporter();

	void SetRelease(const FString& release);
	void SetEnvironment(const FString& environment);
	void SetUser(TSharedPtr<SentryUserDesktop> user);
	void RemoveUser();
	void SetContext(const FString& key, const TMap<FString, FString>& values);
	void SetTag(const FString& key, const FString& value);
	void RemoveTag(const FString& key);

protected:
	void UpdateCrashReporterConfig();

private:
	TSharedPtr<FJsonObject> crashReporterConfig;
};

#endif