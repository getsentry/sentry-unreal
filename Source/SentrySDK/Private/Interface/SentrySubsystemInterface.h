// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"

class USentrySettings;
class USentryEvent;
class USentryId;
class USentryUserFeedback;
class USentryUser;

class ISentrySubsystem
{
public:
	virtual ~ISentrySubsystem() = default;

	virtual void InitWithSettings(const USentrySettings* settings) = 0;
	virtual void Close() = 0;
	virtual void AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level) = 0;
	virtual USentryId* CaptureMessage(const FString& message, ESentryLevel level) = 0;
	virtual USentryId* CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level) = 0;
	virtual USentryId* CaptureEvent(USentryEvent* event) = 0;
	virtual USentryId* CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure) = 0;
	virtual void CaptureUserFeedback(USentryUserFeedback* userFeedback) = 0;
	virtual void SetUser(USentryUser* user) = 0;
	virtual void RemoveUser() = 0;
};