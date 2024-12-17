// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.h"

class ISentryBreadcrumb;
class ISentryEvent;
class ISentryUserFeedback;
class ISentryUser;
class ISentryTransaction;
class ISentryTransactionContext;
class ISentryId;
class ISentryScope;

class USentrySettings;
class USentryBeforeSendHandler;
class USentryTraceSampler;

DECLARE_DELEGATE_OneParam(FSentryScopeDelegate, TSharedPtr<ISentryScope>);

class ISentrySubsystem
{
public:
	virtual ~ISentrySubsystem() = default;

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler) = 0;
	virtual void Close() = 0;
	virtual bool IsEnabled() = 0;
	virtual ESentryCrashedLastRun IsCrashedLastRun() = 0;
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) = 0;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level) = 0;
	virtual void ClearBreadcrumbs() = 0;
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) = 0;
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onConfigureScope, ESentryLevel level) = 0;
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) = 0;
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope) = 0;
	virtual TSharedPtr<ISentryId> CaptureException(const FString& type, const FString& message, int32 framesToSkip = 0) = 0;
	virtual TSharedPtr<ISentryId> CaptureAssertion(const FString& type, const FString& message) = 0;
	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) = 0;
	virtual void CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback) = 0;
	virtual void SetUser(TSharedPtr<ISentryUser> user) = 0;
	virtual void RemoveUser() = 0;
	virtual void ConfigureScope(const FSentryScopeDelegate& onConfigureScope) = 0;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) = 0;
	virtual void SetTag(const FString& key, const FString& value) = 0;
	virtual void RemoveTag(const FString& key) = 0;
	virtual void SetLevel(ESentryLevel level) = 0;
	virtual void StartSession() = 0;
	virtual void EndSession() = 0;
	virtual TSharedPtr<ISentryTransaction> StartTransaction(const FString& name, const FString& operation) = 0;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context) = 0;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp) = 0;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options) = 0;
	virtual TSharedPtr<ISentryTransactionContext> ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders) = 0;
};