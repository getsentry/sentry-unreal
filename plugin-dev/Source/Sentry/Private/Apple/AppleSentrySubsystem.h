// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySubsystemInterface.h"

class FAppleSentrySubsystem : public ISentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;
	virtual bool IsEnabled() override;
	virtual ESentryCrashedLastRun IsCrashedLastRun() override;
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level) override;
	virtual void ClearBreadcrumbs() override;
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope) override;
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) override;
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope) override;
	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override;
	virtual void CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback) override;
	virtual void SetUser(TSharedPtr<ISentryUser> user) override;
	virtual void RemoveUser() override;
	virtual void ConfigureScope(const FSentryScopeDelegate& onConfigureScope) override;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual void StartSession() override;
	virtual void EndSession() override;
	virtual TSharedPtr<ISentryTransaction> StartTransaction(const FString& name, const FString& operation) override;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context) override;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp) override;
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options) override;
	virtual TSharedPtr<ISentryTransactionContext> ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders) override;

	virtual FString TryCaptureScreenshot() const { return FString(); };

protected:
	void UploadAttachmentForEvent(TSharedPtr<ISentryId> eventId, const FString& filePath, const FString& name, bool deleteAfterUpload = false) const;

	void UploadScreenshotForEvent(TSharedPtr<ISentryId> eventId, const FString& screenshotPath) const;
	void UploadGameLogForEvent(TSharedPtr<ISentryId> eventId, const FString& logFilePath) const;

	virtual FString GetScreenshotPath() const;
	virtual FString GetLatestScreenshot() const;
	virtual FString GetGameLogPath() const { return FString(); };
	virtual FString GetLatestGameLog() const { return FString(); };

protected:
	bool isScreenshotAttachmentEnabled = false;
	bool isGameLogAttachmentEnabled = false;
};
