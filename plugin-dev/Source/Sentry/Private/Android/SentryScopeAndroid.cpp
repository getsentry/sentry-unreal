// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeAndroid.h"

#include "SentryBreadcrumbAndroid.h"
#include "SentryAttachmentAndroid.h"

#include "SentryBreadcrumb.h"
#include "SentryAttachment.h"

#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryScopeAndroid::SentryScopeAndroid()
	: FSentryJavaObjectWrapper(GetClassName(), "(Lio/sentry/SentryOptions;)V",
		SentryMethodCallAndroid::CallStaticObjectMethod("io/sentry/unreal/SentryBridgeJava", "getOptions", "()Lio/sentry/SentryOptions;"))
{
	SetupClassMethods();
}

SentryScopeAndroid::SentryScopeAndroid(jobject scope)
	: FSentryJavaObjectWrapper(GetClassName(), scope)
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

FSentryJavaClass SentryScopeAndroid::GetClassName()
{
	return FSentryJavaClass{ "io/sentry/Scope", ESentryJavaClassType::External };
}

void SentryScopeAndroid::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = StaticCastSharedPtr<SentryBreadcrumbAndroid>(breadcrumb->GetNativeImpl());
	CallMethod<void>(AddBreadcrumbMethod, breadcrumbAndroid->GetJObject());
}

void SentryScopeAndroid::ClearBreadcrumbs()
{
	CallMethod<void>(ClearBreadcrumbsMethod);
}

void SentryScopeAndroid::AddAttachment(USentryAttachment* attachment)
{
	TSharedPtr<SentryAttachmentAndroid> attachmentAndroid = StaticCastSharedPtr<SentryAttachmentAndroid>(attachment->GetNativeImpl());
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}

void SentryScopeAndroid::ClearAttachments()
{
	CallMethod<void>(ClearAttachmentsMethod);
}

void SentryScopeAndroid::SetTagValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagValueMethod, *FJavaClassObject::GetJString(key), *FJavaClassObject::GetJString(value));
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
	CallMethod<void>(RemoveTagMethod, *FJavaClassObject::GetJString(key));
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
	auto tags = NewSentryScopedJavaObject(CallMethod<jobject>(GetTagsMethod));
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
	auto fingerprint = NewSentryScopedJavaObject(CallMethod<jobject>(GetFingerprintMethod));
	return SentryConvertorsAndroid::StringListToUnreal(*fingerprint);
}

void SentryScopeAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel SentryScopeAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}

void SentryScopeAndroid::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	CallMethod<void>(SetContextMethod, *FJavaClassObject::GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentryScopeAndroid::RemoveContext(const FString& key)
{
	CallMethod<void>(RemoveContextMethod, *FJavaClassObject::GetJString(key));
}

void SentryScopeAndroid::SetExtraValue(const FString& key, const FString& value)
{
	CallMethod<void>(SetExtraValueMethod, *FJavaClassObject::GetJString(key), *FJavaClassObject::GetJString(value));
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
	CallMethod<void>(RemoveExtraMethod, *FJavaClassObject::GetJString(key));
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
	auto extras = NewSentryScopedJavaObject(CallMethod<jobject>(GetExtrasMethod));
	return SentryConvertorsAndroid::StringMapToUnreal(*extras);
}

void SentryScopeAndroid::Clear()
{
	CallMethod<void>(ClearMethod);
}