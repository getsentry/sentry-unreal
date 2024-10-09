// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "HAL/CriticalSection.h"

#include "Interface/SentrySubsystemInterface.h"

class SentryScopeDesktop;
class SentryCrashReporter;

#if USE_SENTRY_NATIVE

class SentrySubsystemDesktop : public ISentrySubsystem
{
public:
	SentrySubsystemDesktop();

	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler) override;
	virtual void Close() override;
	virtual bool IsEnabled() override;
	virtual ESentryCrashedLastRun IsCrashedLastRun() override;
	virtual void AddBreadcrumb(USentryBreadcrumb* breadcrumb) override;
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level) override;
	virtual void ClearBreadcrumbs() override;
	virtual USentryId* CaptureMessage(const FString& message, ESentryLevel level) override;
	virtual USentryId* CaptureMessageWithScope(const FString& message, const FConfigureScopeNativeDelegate& onScopeConfigure, ESentryLevel level) override;
	virtual USentryId* CaptureEvent(USentryEvent* event) override;
	virtual USentryId* CaptureEventWithScope(USentryEvent* event, const FConfigureScopeNativeDelegate& onScopeConfigure) override;
	virtual USentryId* CaptureException(const FString& type, const FString& message, int32 framesToSkip) override;
	virtual USentryId* CaptureAssertion(const FString& type, const FString& message) override;
	virtual USentryId* CaptureEnsure(const FString& type, const FString& message) override;
	virtual void CaptureUserFeedback(USentryUserFeedback* userFeedback) override;
	virtual void SetUser(USentryUser* user) override;
	virtual void RemoveUser() override;
	virtual void ConfigureScope(const FConfigureScopeNativeDelegate& onConfigureScope) override;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual void StartSession() override;
	virtual void EndSession() override;
	virtual USentryTransaction* StartTransaction(const FString& name, const FString& operation) override;
	virtual USentryTransaction* StartTransactionWithContext(USentryTransactionContext* context) override;
	virtual USentryTransaction* StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options) override;
	virtual USentryTransactionContext* ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders) override;

	USentryBeforeSendHandler* GetBeforeSendHandler();

	void TryCaptureScreenshot() const;

	TSharedPtr<SentryScopeDesktop> GetCurrentScope();

private:
	FString GetHandlerPath() const;
	FString GetDatabasePath() const;
	FString GetScreenshotPath() const;

	USentryBeforeSendHandler* beforeSend;

	TSharedPtr<SentryCrashReporter> crashReporter;

	TArray<TSharedPtr<SentryScopeDesktop>> scopeStack;

	bool isEnabled;

	bool isStackTraceEnabled;
	bool isScreenshotAttachmentEnabled;

	FCriticalSection CriticalSection;

	FString databaseParentPath;
};

#endif
