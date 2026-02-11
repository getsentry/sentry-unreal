// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryLogInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryLog : public ISentryLog
{
public:
	FGenericPlatformSentryLog();
	FGenericPlatformSentryLog(sentry_value_t log);
	FGenericPlatformSentryLog(const FString& body, ESentryLevel level);
	virtual ~FGenericPlatformSentryLog() override = default;

	sentry_value_t GetNativeObject();

	virtual void SetBody(const FString& body) override;
	virtual FString GetBody() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override;
	virtual FSentryVariant GetAttribute(const FString& key) const override;
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const override;
	virtual void RemoveAttribute(const FString& key) override;
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) override;

private:
	sentry_value_t Log;
};

typedef FGenericPlatformSentryLog FPlatformSentryLog;

#endif // USE_SENTRY_NATIVE