// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryScopeInterface.h"

#include "Infrastructure/SentryJavaObjectWrapper.h"

class SentryScopeAndroid : public ISentryScope, public FSentryJavaObjectWrapper
{
public:
	SentryScopeAndroid();
	SentryScopeAndroid(jobject scope);

	void SetupClassMethods();

	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void ClearBreadcrumbs() override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void ClearAttachments() override;
	virtual void SetTagValue(const FString& key, const FString& value) override;
	virtual FString GetTagValue(const FString& key) const override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual TMap<FString, FString> GetTags() const override;
	virtual void SetDist(const FString& dist) override;
	virtual FString GetDist() const override;
	virtual void SetEnvironment(const FString& environment) override;
	virtual FString GetEnvironment() const override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
	virtual void SetContext(const FString& key, const TMap<FString, FString>& values) override;
	virtual void RemoveContext(const FString& key) override;
	virtual void SetExtraValue(const FString& key, const FString& value) override;
	virtual FString GetExtraValue(const FString& key) const override;
	virtual void RemoveExtra(const FString& key) override;
	virtual void SetExtras(const TMap<FString, FString>& extras) override;
	virtual TMap<FString, FString> GetExtras() const override;
	virtual void Clear() override;

private:
	FSentryJavaMethod AddBreadcrumbMethod;
	FSentryJavaMethod ClearBreadcrumbsMethod;
	FSentryJavaMethod AddAttachmentMethod;
	FSentryJavaMethod ClearAttachmentsMethod;
	FSentryJavaMethod SetTagValueMethod;
	FSentryJavaMethod RemoveTagMethod;
	FSentryJavaMethod GetTagsMethod;
	FSentryJavaMethod SetFingerprintMethod;
	FSentryJavaMethod GetFingerprintMethod;
	FSentryJavaMethod SetLevelMethod;
	FSentryJavaMethod GetLevelMethod;
	FSentryJavaMethod SetContextMethod;
	FSentryJavaMethod RemoveContextMethod;
	FSentryJavaMethod SetExtraValueMethod;
	FSentryJavaMethod GetExtraValueMethod;
	FSentryJavaMethod RemoveExtraMethod;
	FSentryJavaMethod GetExtrasMethod;
	FSentryJavaMethod ClearMethod;
};
