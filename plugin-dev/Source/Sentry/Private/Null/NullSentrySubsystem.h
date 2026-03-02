// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentrySubsystemInterface.h"

class FNullSentrySubsystem : public ISentrySubsystem
{
public:
	virtual ~FNullSentrySubsystem() override = default;

	virtual void InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers) override {}
	virtual void Close() override {}
	virtual bool IsEnabled() override { return false; }
	virtual ESentryCrashedLastRun IsCrashedLastRun() override { return ESentryCrashedLastRun::NotEvaluated; }
	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override {}
	virtual void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level) override {}
	virtual void AddLog(const FString& Message, ESentryLevel Level, const TMap<FString, FSentryVariant>& Attributes) override {}
	virtual void AddCount(const FString& Key, int32 Value, const TMap<FString, FSentryVariant>& Attributes) override {}
	virtual void AddDistribution(const FString& Key, float Value, const FString& Unit, const TMap<FString, FSentryVariant>& Attributes) override {}
	virtual void AddGauge(const FString& Key, float Value, const FString& Unit, const TMap<FString, FSentryVariant>& Attributes) override {}
	virtual void ClearBreadcrumbs() override {}
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual void RemoveAttachment(TSharedPtr<ISentryAttachment> attachment) override {}
	virtual void ClearAttachments() override {}
	virtual TSharedPtr<ISentryId> CaptureMessage(const FString& message, ESentryLevel level) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEvent(TSharedPtr<ISentryEvent> event) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureEnsure(const FString& type, const FString& message) override { return nullptr; }
	virtual TSharedPtr<ISentryId> CaptureHang(uint32 HungThreadId) override { return nullptr; }
	virtual void CaptureFeedback(TSharedPtr<ISentryFeedback> feedback) override {}
	virtual void SetUser(TSharedPtr<ISentryUser> user) override {}
	virtual void RemoveUser() override {}
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override {}
	virtual void SetTag(const FString& key, const FString& value) override {}
	virtual void RemoveTag(const FString& key) override {}
	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override {}
	virtual void RemoveAttribute(const FString& key) override {}
	virtual void SetLevel(ESentryLevel level) override {}
	virtual void StartSession() override {}
	virtual void EndSession() override {}
	virtual void GiveUserConsent() override {}
	virtual void RevokeUserConsent() override {}
	virtual EUserConsent GetUserConsent() const override { return EUserConsent::Unknown; }
	virtual bool IsUserConsentRequired() const override { return false; }
	virtual TSharedPtr<ISentryTransaction> StartTransaction(const FString& name, const FString& operation, bool bindToScope) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context, bool bindToScope) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp, bool bindToScope) override { return nullptr; }
	virtual TSharedPtr<ISentryTransaction> StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const FSentryTransactionOptions& options) override { return nullptr; }
	virtual TSharedPtr<ISentryTransactionContext> ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders) override { return nullptr; }

	virtual void HandleAssert() override {}
};

typedef FNullSentrySubsystem FPlatformSentrySubsystem;
