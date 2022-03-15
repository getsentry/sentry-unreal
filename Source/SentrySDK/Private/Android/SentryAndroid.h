// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryScope.h"

class USentrySettings;

class SentryAndroid
{
public:
	static void InitWithSettings(const USentrySettings* settings);
	static FString CaptureMessage(const FString& message, ESentryLevel level);
	static FString CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level);

private:
	static const ANSICHAR* SentryJavaClassName;
};
