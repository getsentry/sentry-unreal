// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJava.h"

#include "Interface/SentryScopeInterface.h"

class SentryScopeAndroid : public ISentryScope, public FJavaClassObject
{
public:
	SentryScopeAndroid();
	SentryScopeAndroid(jobject scope);

	static FName GetClassName();

	virtual void AddBreadcrumb(USentryBreadcrumb* breadcrumb) override;
	virtual void ClearBreadcrumbs() override;
	virtual void AddAttachment(USentryAttachment* attachment) override;
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
	FJavaClassMethod AddBreadcrumbMethod;
	FJavaClassMethod ClearBreadcrumbsMethod;
	FJavaClassMethod AddAttachmentMethod;
	FJavaClassMethod ClearAttachmentsMethod;
	FJavaClassMethod SetTagValueMethod;
	FJavaClassMethod RemoveTagMethod;
	FJavaClassMethod GetTagsMethod;
	FJavaClassMethod SetFingerprintMethod;
	FJavaClassMethod GetFingerprintMethod;
	FJavaClassMethod SetLevelMethod;
	FJavaClassMethod GetLevelMethod;
	FJavaClassMethod SetContextMethod;
	FJavaClassMethod RemoveContextMethod;
	FJavaClassMethod SetExtraValueMethod;
	FJavaClassMethod GetExtraValueMethod;
	FJavaClassMethod RemoveExtraMethod;
	FJavaClassMethod GetExtrasMethod;
	FJavaClassMethod ClearMethod;
};
