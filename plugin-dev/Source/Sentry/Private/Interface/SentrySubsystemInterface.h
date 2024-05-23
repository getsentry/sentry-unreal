// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"

class USentrySettings;
class USentryBreadcrumb;
class USentryEvent;
class USentryId;
class USentryUserFeedback;
class USentryUser;
class USentryBeforeSendHandler;
class USentryTransaction;
class USentryTraceSampler;
class USentryTransactionContext;

class ISentrySubsystem
{
public:
	virtual ~ISentrySubsystem() = default;

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler) = 0;
	virtual void Close() = 0;
	virtual bool IsEnabled() = 0;
	virtual ESentryCrashedLastRun IsCrashedLastRun() = 0;
	virtual void AddBreadcrumb(USentryBreadcrumb* breadcrumb) = 0;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level) = 0;
	virtual void ClearBreadcrumbs() = 0;
	virtual USentryId* CaptureMessage(const FString& message, ESentryLevel level) = 0;
	virtual USentryId* CaptureMessageWithScope(const FString& message, const FConfigureScopeNativeDelegate& onConfigureScope, ESentryLevel level) = 0;
	virtual USentryId* CaptureEvent(USentryEvent* event) = 0;
	virtual USentryId* CaptureEventWithScope(USentryEvent* event, const FConfigureScopeNativeDelegate& onConfigureScope) = 0;
	virtual USentryId* CaptureException(const FString& type, const FString& message, int32 framesToSkip = 0) = 0;
	virtual USentryId* CaptureAssertion(const FString& type, const FString& message) = 0;
	virtual USentryId* CaptureEnsure(const FString& type, const FString& message) = 0;
	virtual void CaptureUserFeedback(USentryUserFeedback* userFeedback) = 0;
	virtual void SetUser(USentryUser* user) = 0;
	virtual void RemoveUser() = 0;
	virtual void ConfigureScope(const FConfigureScopeNativeDelegate& onConfigureScope) = 0;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) = 0;
	virtual void SetTag(const FString& key, const FString& value) = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual void StartSession() = 0;
	virtual void EndSession() = 0;
	virtual USentryTransaction* StartTransaction(const FString& name, const FString& operation) = 0;
	virtual USentryTransaction* StartTransactionWithContext(USentryTransactionContext* context) = 0;
	virtual USentryTransaction* StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options) = 0;
};