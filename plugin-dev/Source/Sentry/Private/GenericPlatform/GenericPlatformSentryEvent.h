// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryEventInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryEvent : public ISentryEvent
{
public:
	FGenericPlatformSentryEvent();
	FGenericPlatformSentryEvent(sentry_value_t event, bool isCrash = false);
	virtual ~FGenericPlatformSentryEvent() override;

	sentry_value_t GetNativeObject();

	virtual TSharedPtr<ISentryId> GetId() const override;
	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual FString GetTag(const FString& key) const override;
	virtual bool TryGetTag(const FString& key, FString& value) const override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual TMap<FString, FString> GetTags() const override;
	virtual void SetContext(const FString& key, const TMap<FString, FSentryVariant>& values) override;
	virtual TMap<FString, FSentryVariant> GetContext(const FString& key) const override;
	virtual bool TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const override;
	virtual void RemoveContext(const FString& key) override;
	virtual void SetExtra(const FString& key, const FSentryVariant& value) override;
	virtual FSentryVariant GetExtra(const FString& key) const override;
	virtual bool TryGetExtra(const FString& key, FSentryVariant& value) const override;
	virtual void RemoveExtra(const FString& key) override;
	virtual void SetExtras(const TMap<FString, FSentryVariant>& extras) override;
	virtual TMap<FString, FSentryVariant> GetExtras() const override;
	virtual bool IsCrash() const override;
	virtual bool IsAnr() const override;

private:
	sentry_value_t Event;

	bool IsCrashEvent;
};

typedef FGenericPlatformSentryEvent FPlatformSentryEvent;

#endif
