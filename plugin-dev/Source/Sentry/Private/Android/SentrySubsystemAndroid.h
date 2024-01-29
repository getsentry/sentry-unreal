// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySubsystemInterface.h"

class SentrySubsystemAndroid : public ISentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;
	virtual bool IsEnabled() override;
	virtual ESentryCrashedLastRun IsCrashedLastRun() override;
	virtual void AddBreadcrumb(USentryBreadcrumb* breadcrumb) override;
	virtual void ClearBreadcrumbs() override;
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
	virtual void SetLevel(ESentryLevel level) override;
	virtual void StartSession() override;
	virtual void EndSession() override;
	virtual USentryTransaction* StartTransaction(const FString& name, const FString& operation) override;
	virtual USentryTransaction* StartTransactionWithContext(USentryTransactionContext* context) override;
	virtual USentryTransaction* StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options) override;
};
