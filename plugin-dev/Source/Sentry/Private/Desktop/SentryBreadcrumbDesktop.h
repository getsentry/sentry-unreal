// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryBreadcrumbInterface.h"

#if USE_SENTRY_NATIVE

class SentryBreadcrumbDesktop : public ISentryBreadcrumb
{
public:
	SentryBreadcrumbDesktop();
	SentryBreadcrumbDesktop(sentry_value_t breadcrumb);
	virtual ~SentryBreadcrumbDesktop() override;

	sentry_value_t GetNativeObject();

	virtual void SetMessage(const FString& message) override;
	virtual FString GetMessage() const override;
	virtual void SetType(const FString& type) override;
	virtual FString GetType() const override;
	virtual void SetCategory(const FString& category) override;
	virtual FString GetCategory() const override;
	virtual void SetData(const TMap<FString, FString>& data) override;
	virtual TMap<FString, FString> GetData() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;

private:
	sentry_value_t BreadcrumbDesktop;
};

#endif
