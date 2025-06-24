// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryScopeInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryScope : public ISentryScope, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryScope();
	FAndroidSentryScope(jobject scope);

	void SetupClassMethods();

	virtual void AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb) override;
	virtual void ClearBreadcrumbs() override;
	virtual void AddAttachment(TSharedPtr<ISentryAttachment> attachment) override;
	virtual void ClearAttachments() override;
	virtual void SetTag(const FString& key, const FString& value) override;
	virtual FString GetTag(const FString& key) const override;
	virtual bool TryGetTag(const FString& key, FString& value) const override;
	virtual void RemoveTag(const FString& key) override;
	virtual void SetTags(const TMap<FString, FString>& tags) override;
	virtual TMap<FString, FString> GetTags() const override;
	virtual void SetFingerprint(const TArray<FString>& fingerprint) override;
	virtual TArray<FString> GetFingerprint() const override;
	virtual void SetLevel(ESentryLevel level) override;
	virtual ESentryLevel GetLevel() const override;
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

typedef FAndroidSentryScope FPlatformSentryScope;
