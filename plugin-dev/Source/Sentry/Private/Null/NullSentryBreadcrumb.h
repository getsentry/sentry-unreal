// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryBreadcrumbInterface.h"

class ISentryAttachment;

class FNullSentryBreadcrumb final : public ISentryBreadcrumb
{
public:
	virtual ~FNullSentryBreadcrumb() override = default;

	virtual void SetMessage(const FString& message) override {}
	virtual FString GetMessage() const override { return TEXT(""); }
	virtual void SetType(const FString& type) override {}
	virtual FString GetType() const override { return TEXT(""); }
	virtual void SetCategory(const FString& category) override {}
	virtual FString GetCategory() const override { return TEXT(""); }
	virtual void SetData(const TMap<FString, FSentryVariant>& data) override {}
	virtual TMap<FString, FSentryVariant> GetData() const override { return {}; }
	virtual void SetLevel(ESentryLevel level) override {}
	virtual ESentryLevel GetLevel() const override { return ESentryLevel::Debug; }
};

typedef FNullSentryBreadcrumb FPlatformSentryBreadcrumb;
