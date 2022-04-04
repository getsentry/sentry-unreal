// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryScope.h"
#include "Interface/SentrySubsystemInterface.h"

class USentrySettings;
class USentryEvent;

class SentrySubsystemAndroid : public ISentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings) override;
	virtual void AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level) override;
	virtual FString CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual FString CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level) override;
	virtual FString CaptureEvent(USentryEvent* event) override;
	virtual FString CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure) override;

private:
	static const ANSICHAR* SentryJavaClassName;
};
