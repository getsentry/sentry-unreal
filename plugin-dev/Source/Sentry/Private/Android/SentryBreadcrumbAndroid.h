// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryBreadcrumbInterface.h"

#include "Infrastructure/SentryJavaClassWrapper.h"

class SentryBreadcrumbAndroid : public ISentryBreadcrumb, public FSentryJavaClassWrapper
{
public:
	SentryBreadcrumbAndroid();
	SentryBreadcrumbAndroid(jobject breadcrumb);

	void SetupClassMethods();

	static FName GetClassName();

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
	FSentryJavaClassMethod SetMessageMethod;
	FSentryJavaClassMethod GetMessageMethod;
	FSentryJavaClassMethod SetTypeMethod;
	FSentryJavaClassMethod GetTypeMethod;
	FSentryJavaClassMethod SetCategoryMethod;
	FSentryJavaClassMethod GetCategoryMethod;
	FSentryJavaClassMethod SetDataMethod;
	FSentryJavaClassMethod GetDataMethod;
	FSentryJavaClassMethod SetLevelMethod;
	FSentryJavaClassMethod GetLevelMethod;
};
