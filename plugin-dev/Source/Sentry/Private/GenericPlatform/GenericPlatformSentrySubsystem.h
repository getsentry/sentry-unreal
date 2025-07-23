// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentrySubsystemInterface.h"

#include "HAL/CriticalSection.h"

class FGenericPlatformSentryAttachment;
class FGenericPlatformSentryScope;
class FGenericPlatformSentryCrashReporter;

#if USE_SENTRY_NATIVE

class FGenericPlatformSentrySubsystem : public ISentrySubsystem
{
public:
	FGenericPlatformSentrySubsystem();

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;
	virtual bool IsEnabled() override;
	virtual ESentryCrashedLastRun IsCrashedLastRun() override;
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level) override;
	virtual void ClearBreadcrumbs() override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void RemoveAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void ClearAttachments() override;
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope) override;
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) override;
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure) override;
	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override;
	virtual void CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback) override;
	virtual void SetUser(TSharedPtr<ISentryUser> user) override;
	virtual void RemoveUser() override;
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override;
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

	virtual void HandleAssert() override {}

	USentryBeforeSendHandler* GetBeforeSendHandler();
	USentryBeforeBreadcrumbHandler* GetBeforeBreadcrumbHandler();

	void TryCaptureScreenshot();
	void TryCaptureGpuDump();

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) {}
	virtual void ConfigureDatabasePath(sentry_options_t* Options) {}
	virtual void ConfigureCertsPath(sentry_options_t* Options) {}
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) {}
	virtual void ConfigureNetworkConnectFunc(sentry_options_t* Options) {}

	FString GetHandlerPath() const;
	FString GetDatabasePath() const;
	FString GetScreenshotPath() const;
	virtual FString GetHandlerExecutableName() const { return TEXT("invalid"); }

	virtual sentry_value_t OnBeforeSend(sentry_value_t event, void* hint, void* closure, bool isCrash);
	virtual sentry_value_t OnBeforeBreadcrumb(sentry_value_t breadcrumb, void* hint, void* closure);
	virtual sentry_value_t OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure);

	void InitCrashReporter(const FString& release, const FString& environment);

	virtual void AddFileAttachment(TSharedPtr<ISentryAttachment> attachment);
	virtual void AddByteAttachment(TSharedPtr<ISentryAttachment> attachment);

	TArray<TSharedPtr<FGenericPlatformSentryAttachment>> attachments;

private:
	/**
	 * Static wrappers that are passed to the Sentry library.
	 */
	static sentry_value_t HandleBeforeSend(sentry_value_t event, void* hint, void* closure);
	static sentry_value_t HandleBeforeBreadcrumb(sentry_value_t breadcrumb, void* hint, void* closure);
	static sentry_value_t HandleOnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure);

	USentryBeforeSendHandler* beforeSend;
	USentryBeforeBreadcrumbHandler* beforeBreadcrumb;

	TSharedPtr<FGenericPlatformSentryCrashReporter> crashReporter;

	bool isEnabled;

	bool isStackTraceEnabled;
	bool isPiiAttachmentEnabled;
	bool isScreenshotAttachmentEnabled;
	bool isGpuDumpAttachmentEnabled;

	FString databaseParentPath;
};

#endif
