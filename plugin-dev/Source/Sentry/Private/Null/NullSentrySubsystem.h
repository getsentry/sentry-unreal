// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySubsystemInterface.h"

class FNullSentrySubsystem : public ISentrySubsystem
{
public:
	virtual ~FNullSentrySubsystem() override = default;

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler) override {}
	virtual void Close() override {}
	virtual bool IsEnabled() override { return false; }
	virtual ESentryCrashedLastRun IsCrashedLastRun() override { return ESentryCrashedLastRun::NotEvaluated; }
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override {}
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level) override {}
	virtual void ClearBreadcrumbs() override {}
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual void RemoveAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual void ClearAttachments() override {}
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override { return nullptr; }
	virtual void CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback) override {}
	virtual void SetUser(TSharedPtr<ISentryUser> user) override {}
	virtual void RemoveUser() override {}
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override {}
	virtual void SetTag(const FString& key, const FString& value) override {}
	virtual void RemoveTag(const FString& key) override {}
	virtual void SetLevel(ESentryLevel level) override {}
	virtual void StartSession() override {}
	virtual void EndSession() override {}
	virtual TSharedPtr<ISentryTransaction> StartTransaction(const FString& name, const FString& operation) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options) override { return nullptr; }
	virtual TSharedPtr<ISentryTransactionContext> ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders) override { return nullptr; }

	virtual void HandleAssert() override {}
};

typedef FNullSentrySubsystem FPlatformSentrySubsystem;
