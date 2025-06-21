// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryBreadcrumbInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryBreadcrumb : public ISentryBreadcrumb
{
public:
	FGenericPlatformSentryBreadcrumb();
	FGenericPlatformSentryBreadcrumb(sentry_value_t breadcrumb);
	virtual ~FGenericPlatformSentryBreadcrumb() override = default;

	sentry_value_t GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetType(const FString& type) override;
	virtual FString GetType() const override;
	virtual void SetCategory(const FString& category) override;
	virtual FString GetCategory() const override;
	virtual void SetData(const TMap<FString, FSentryVariant>& data) override;
	virtual TMap<FString, FSentryVariant> GetData() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	sentry_value_t Breadcrumb;
};

typedef FGenericPlatformSentryBreadcrumb FPlatformSentryBreadcrumb;

#endif
