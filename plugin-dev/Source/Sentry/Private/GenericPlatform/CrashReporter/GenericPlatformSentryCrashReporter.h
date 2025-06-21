// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryVariant.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryUser;

class FJsonObject;

class FGenericPlatformSentryCrashReporter
{
public:
	FGenericPlatformSentryCrashReporter();

	void SetRelease(const FString& release);
	void SetEnvironment(const FString& environment);
	void SetUser(TSharedPtr<FGenericPlatformSentryUser> user);
	void RemoveUser();
	void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values);
	void SetTag(const FString& key, const FString& value);
	void RemoveTag(const FString& key);

protected:
	void UpdateCrashReporterConfig();

private:
	TSharedPtr<FJsonObject> crashReporterConfig;
};

#endif