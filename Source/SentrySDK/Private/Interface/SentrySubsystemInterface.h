// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"

class USentrySettings;
class USentryEvent;

class ISentrySubsystem
{
public:
	virtual ~ISentrySubsystem() = default;

	virtual void InitWithSettings(const USentrySettings* settings) = 0;
	virtual void AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level) = 0;
	virtual FString CaptureMessage(const FString& message, ESentryLevel level) = 0;
	virtual FString CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level) = 0;
	virtual FString CaptureEvent(USentryEvent* event) = 0;
	virtual FString CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure) = 0;
};