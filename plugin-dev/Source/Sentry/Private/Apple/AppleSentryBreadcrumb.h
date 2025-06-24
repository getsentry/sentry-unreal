// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryBreadcrumbInterface.h"

@class SentryBreadcrumb;

class FAppleSentryBreadcrumb : public ISentryBreadcrumb
{
public:
	FAppleSentryBreadcrumb();
	FAppleSentryBreadcrumb(SentryBreadcrumb* breadcrumb);
	virtual ~FAppleSentryBreadcrumb() override;

	SentryBreadcrumb* GetNativeObject();

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
	SentryBreadcrumb* BreadcrumbApple;
};

typedef FAppleSentryBreadcrumb FPlatformSentryBreadcrumb;
