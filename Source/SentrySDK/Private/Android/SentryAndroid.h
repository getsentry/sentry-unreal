// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryScope.h"

class USentrySettings;
class USentryEvent;

class SentryAndroid
{
public:
	static void InitWithSettings(const USentrySettings* settings);
	static void AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level);
	static FString CaptureMessage(const FString& message, ESentryLevel level);
	static FString CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level);
	static FString CaptureEvent(USentryEvent* event);
	static FString CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure);

private:
	static const ANSICHAR* SentryJavaClassName;
};
