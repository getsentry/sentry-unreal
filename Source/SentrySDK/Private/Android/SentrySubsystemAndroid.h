// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySubsystemInterface.h"

class SentrySubsystemAndroid : public ISentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings) override;
	virtual void Close() override;
	virtual void AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level) override;
	virtual USentryId* CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual USentryId* CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level) override;
	virtual USentryId* CaptureEvent(USentryEvent* event) override;
	virtual USentryId* CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope) override;
	virtual void CaptureUserFeedback(USentryUserFeedback* userFeedback) override;
	virtual void SetUser(USentryUser* user) override;
	virtual void RemoveUser() override;
	virtual void ConfigureScope(const FConfigureScopeDelegate& onConfigureScope) override;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;

private:
	static const ANSICHAR* SentryJavaClassName;
	static const ANSICHAR* SentryBridgeJavaClassName;
};
