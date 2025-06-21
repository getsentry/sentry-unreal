// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryBreadcrumbInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryBreadcrumb : public ISentryBreadcrumb, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryBreadcrumb();
	FAndroidSentryBreadcrumb(jobject breadcrumb);

	void SetupClassMethods();

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
	FSentryJavaMethod SetMessageMethod;
	FSentryJavaMethod GetMessageMethod;
	FSentryJavaMethod SetTypeMethod;
	FSentryJavaMethod GetTypeMethod;
	FSentryJavaMethod SetCategoryMethod;
	FSentryJavaMethod GetCategoryMethod;
	FSentryJavaMethod SetDataMethod;
	FSentryJavaMethod GetDataMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
};

typedef FAndroidSentryBreadcrumb FPlatformSentryBreadcrumb;
