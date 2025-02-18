// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/CriticalSection.h"

#include "Interface/SentrySubsystemInterface.h"

class FGenericPlatformSentryScope;
class FGenericPlatformSentryCrashReporter;

#if USE_SENTRY_NATIVE

#include "GenericPlatform/Convenience/SentryInclude.h"

class FGenericPlatformSentrySubsystem : public ISentrySubsystem
{
public:
	FGenericPlatformSentrySubsystem();

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;
	virtual bool IsEnabled() override;
	virtual ESentryCrashedLastRun IsCrashedLastRun() override;
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level) override;
	virtual void ClearBreadcrumbs() override;
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onScopeConfigure, ESentryLevel level) override;
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) override;
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure) override;
	virtual TSharedPtr<ISentryId> CaptureException(const FString& type, const FString& message, int32 framesToSkip) override;
	virtual TSharedPtr<ISentryId> CaptureAssertion(const FString& type, const FString& message) override;
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

	USentryBeforeSendHandler* GetBeforeSendHandler();

	void TryCaptureScreenshot() const;

	FString GetGpuDumpBackupPath() const;

	TSharedPtr<FGenericPlatformSentryScope> GetCurrentScope();

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) {}
	virtual void ConfigureDatabasePath(sentry_options_t* Options) {}
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) {}
	virtual void ConfigureScreenshotAttachment(sentry_options_t* Options) {}
	virtual void ConfigureGpuDumpAttachment(sentry_options_t* Options) {}

	FString GetHandlerPath() const;
	FString GetDatabasePath() const;
	FString GetScreenshotPath() const;
	virtual int32 GetAssertionFramesToSkip() const override { return 5; }
	virtual int32 GetEnsureFramesToSkip() const override { return 7; }
	virtual FString GetHandlerExecutableName() const { return TEXT("invalid"); }

	virtual sentry_value_t OnBeforeSend(sentry_value_t event, void* hint, void* closure);
	virtual sentry_value_t OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure);

private:
	/**
	 * Static wrappers that are passed to the Sentry library.
	 */
	static sentry_value_t HandleBeforeSend(sentry_value_t event, void* hint, void* closure);
	static sentry_value_t HandleOnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure);

	USentryBeforeSendHandler* beforeSend;

	TSharedPtr<FGenericPlatformSentryCrashReporter> crashReporter;

	TArray<TSharedPtr<FGenericPlatformSentryScope>> scopeStack;

	bool isEnabled;

	bool isStackTraceEnabled;
	bool isPiiAttachmentEnabled;
	bool isScreenshotAttachmentEnabled;

	FCriticalSection CriticalSection;

	FString databaseParentPath;
};

#endif
