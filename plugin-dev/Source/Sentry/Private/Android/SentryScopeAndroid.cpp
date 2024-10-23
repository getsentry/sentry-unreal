// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeAndroid.h"

#include "SentryBreadcrumbAndroid.h"
#include "SentryAttachmentAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryScopeAndroid::SentryScopeAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::Scope, "(Lio/sentry/SentryOptions;)V",
		*FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getOptions", "()Lio/sentry/SentryOptions;"))
{
	SetupClassMethods();
}

SentryScopeAndroid::SentryScopeAndroid(jobject scope)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Scope, scope)
{
	SetupClassMethods();
}

void SentryScopeAndroid::SetupClassMethods()
{
	AddBreadcrumbMethod = GetMethod("addBreadcrumb", "(Lio/sentry/Breadcrumb;)V");
	ClearBreadcrumbsMethod = GetMethod("clearBreadcrumbs", "()V");
	AddAttachmentMethod = GetMethod("addAttachment", "(Lio/sentry/Attachment;)V");
	ClearAttachmentsMethod = GetMethod("clearAttachments", "()V");
	SetTagValueMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	RemoveTagMethod = GetMethod("removeTag", "(Ljava/lang/String;)V");
	GetTagsMethod = GetMethod("getTags", "()Ljava/util/Map;");
	SetFingerprintMethod = GetMethod("setFingerprint", "(Ljava/util/List;)V");
	GetFingerprintMethod = GetMethod("getFingerprint", "()Ljava/util/List;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
	SetContextMethod = GetMethod("setContexts", "(Ljava/lang/String;Ljava/lang/Object;)V");
	RemoveContextMethod = GetMethod("removeContexts", "(Ljava/lang/String;)V");
	SetExtraValueMethod = GetMethod("setExtra", "(Ljava/lang/String;Ljava/lang/String;)V");
	RemoveExtraMethod = GetMethod("removeExtra", "(Ljava/lang/String;)V");
	GetExtrasMethod = GetMethod("getExtras", "()Ljava/util/Map;");
	ClearMethod = GetMethod("clear", "()V");
}

void SentryScopeAndroid::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = StaticCastSharedPtr<SentryBreadcrumbAndroid>(breadcrumb);
	CallMethod<void>(AddBreadcrumbMethod, breadcrumbAndroid->GetJObject());
}

void SentryScopeAndroid::ClearBreadcrumbs()
{
	CallMethod<void>(ClearBreadcrumbsMethod);
}

void SentryScopeAndroid::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<SentryAttachmentAndroid> attachmentAndroid = StaticCastSharedPtr<SentryAttachmentAndroid>(attachment);
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}

void SentryScopeAndroid::ClearAttachments()
{
	CallMethod<void>(ClearAttachmentsMethod);
}

void SentryScopeAndroid::SetTagValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagValueMethod, *GetJString(key), *GetJString(value));
}

FString SentryScopeAndroid::GetTagValue(const FString& key) const
{
	TMap<FString, FString> tags = GetTags();
	FString* tagValue = tags.Find(key);

	if (!tagValue)
		return FString();

	return *tagValue;
}

void SentryScopeAndroid::RemoveTag(const FString& key)
{
	CallMethod<void>(RemoveTagMethod, *GetJString(key));
}

void SentryScopeAndroid::SetTags(const TMap<FString, FString>& tags)
{
	for (const auto& tag : tags)
	{
		SetTagValue(tag.Key, tag.Value);
	}
}

TMap<FString, FString> SentryScopeAndroid::GetTags() const
{
	auto tags = CallObjectMethod<jobject>(GetTagsMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*tags);
}

void SentryScopeAndroid::SetDist(const FString& dist)
{
	SetTagValue("dist", dist);
}

FString SentryScopeAndroid::GetDist() const
{
	return GetTagValue("dist");
}

void SentryScopeAndroid::SetEnvironment(const FString& environment)
{
	SetTagValue("environment", environment);
}

FString SentryScopeAndroid::GetEnvironment() const
{
	return GetTagValue("environment");
}

void SentryScopeAndroid::SetFingerprint(const TArray<FString>& fingerprint)
{
	CallMethod<void>(SetFingerprintMethod, SentryConvertorsAndroid::StringArrayToNative(fingerprint)->GetJObject());
}

TArray<FString> SentryScopeAndroid::GetFingerprint() const
{
	auto fingerprint = CallObjectMethod<jobject>(GetFingerprintMethod);
	return SentryConvertorsAndroid::StringListToUnreal(*fingerprint);
}

void SentryScopeAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel SentryScopeAndroid::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}

void SentryScopeAndroid::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetContextMethod, *GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentryScopeAndroid::RemoveContext(const FString& key)
{
	CallMethod<void>(RemoveContextMethod, *GetJString(key));
}

void SentryScopeAndroid::SetExtraValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetExtraValueMethod, *GetJString(key), *GetJString(value));
}

FString SentryScopeAndroid::GetExtraValue(const FString& key) const
{
	TMap<FString, FString> extras = GetTags();
	FString* extraValue = extras.Find(key);

	if (!extraValue)
		return FString();

	return *extraValue;
}

void SentryScopeAndroid::RemoveExtra(const FString& key)
{
	CallMethod<void>(RemoveExtraMethod, *GetJString(key));
}

void SentryScopeAndroid::SetExtras(const TMap<FString, FString>& extras)
{
	for (const auto& extra : extras)
	{
		SetExtraValue(extra.Key, extra.Value);
	}
}

TMap<FString, FString> SentryScopeAndroid::GetExtras() const
{
	auto extras = CallObjectMethod<jobject>(GetExtrasMethod);
	return SentryConvertorsAndroid::StringMapToUnreal(*extras);
}

void SentryScopeAndroid::Clear()
{
	CallMethod<void>(ClearMethod);
}