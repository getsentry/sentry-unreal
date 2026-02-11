// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryLogInterface.h"

class FNullSentryLog final : public ISentryLog
{
public:
	virtual ~FNullSentryLog() override = default;

	virtual void SetBody(const FString& body) override {}
	virtual FString GetBody() const override { return TEXT(""); }
	virtual void SetLevel(ESentryLevel level) override {}
	virtual ESentryLevel GetLevel() const override { return ESentryLevel::Info; }

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override {}
	virtual FSentryVariant GetAttribute(const FString& key) const override { return FSentryVariant(); }
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const override { return false; }
	virtual void RemoveAttribute(const FString& key) override {}
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) override {}
};

typedef FNullSentryLog FPlatformSentryLog;