// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryScope.h"

#include "AndroidSentryAttachment.h"
#include "AndroidSentryBreadcrumb.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryScope::FAndroidSentryScope()
	: FSentryJavaObjectWrapper(SentryJavaClasses::ScopeImpl, "(Lio/sentry/SentryOptions;)V",
		  *FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getOptions", "()Lio/sentry/SentryOptions;"))
{
	SetupClassMethods();
}

FAndroidSentryScope::FAndroidSentryScope(jobject scope)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Scope, scope)
{
	SetupClassMethods();
}

void FAndroidSentryScope::SetupClassMethods()
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

void FAndroidSentryScope::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<FAndroidSentryBreadcrumb> breadcrumbAndroid = StaticCastSharedPtr<FAndroidSentryBreadcrumb>(breadcrumb);
	CallMethod<void>(AddBreadcrumbMethod, breadcrumbAndroid->GetJObject());
}

void FAndroidSentryScope::ClearBreadcrumbs()
{
	CallMethod<void>(ClearBreadcrumbsMethod);
}

void FAndroidSentryScope::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAndroidSentryAttachment> attachmentAndroid = StaticCastSharedPtr<FAndroidSentryAttachment>(attachment);
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}

void FAndroidSentryScope::ClearAttachments()
{
	CallMethod<void>(ClearAttachmentsMethod);
}

void FAndroidSentryScope::SetTagValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagValueMethod, *GetJString(key), *GetJString(value));
}

FString FAndroidSentryScope::GetTagValue(const FString& key) const
{
	TMap<FString, FString> tags = GetTags();
	FString* tagValue = tags.Find(key);

	if (!tagValue)
		return FString();

	return *tagValue;
}

void FAndroidSentryScope::RemoveTag(const FString& key)
{
	CallMethod<void>(RemoveTagMethod, *GetJString(key));
}

void FAndroidSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	for (const auto& tag : tags)
	{
		SetTagValue(tag.Key, tag.Value);
	}
}

TMap<FString, FString> FAndroidSentryScope::GetTags() const
{
	auto tags = CallObjectMethod<jobject>(GetTagsMethod);
	return FAndroidSentryConverters::StringMapToUnreal(*tags);
}

void FAndroidSentryScope::SetFingerprint(const TArray<FString>& fingerprint)
{
	CallMethod<void>(SetFingerprintMethod, FAndroidSentryConverters::StringArrayToNative(fingerprint)->GetJObject());
}

TArray<FString> FAndroidSentryScope::GetFingerprint() const
{
	auto fingerprint = CallObjectMethod<jobject>(GetFingerprintMethod);
	return FAndroidSentryConverters::StringListToUnreal(*fingerprint);
}

void FAndroidSentryScope::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel FAndroidSentryScope::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return FAndroidSentryConverters::SentryLevelToUnreal(*level);
}

void FAndroidSentryScope::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetContextMethod, *GetJString(key), FAndroidSentryConverters::StringMapToNative(values)->GetJObject());
}

void FAndroidSentryScope::RemoveContext(const FString& key)
{
	CallMethod<void>(RemoveContextMethod, *GetJString(key));
}

void FAndroidSentryScope::SetExtraValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetExtraValueMethod, *GetJString(key), *GetJString(value));
}

FString FAndroidSentryScope::GetExtraValue(const FString& key) const
{
	TMap<FString, FString> extras = GetExtras();
	FString* extraValue = extras.Find(key);

	if (!extraValue)
		return FString();

	return *extraValue;
}

void FAndroidSentryScope::RemoveExtra(const FString& key)
{
	CallMethod<void>(RemoveExtraMethod, *GetJString(key));
}

void FAndroidSentryScope::SetExtras(const TMap<FString, FString>& extras)
{
	for (const auto& extra : extras)
	{
		SetExtraValue(extra.Key, extra.Value);
	}
}

TMap<FString, FString> FAndroidSentryScope::GetExtras() const
{
	auto extras = CallObjectMethod<jobject>(GetExtrasMethod);
	return FAndroidSentryConverters::StringMapToUnreal(*extras);
}

void FAndroidSentryScope::Clear()
{
	CallMethod<void>(ClearMethod);
}