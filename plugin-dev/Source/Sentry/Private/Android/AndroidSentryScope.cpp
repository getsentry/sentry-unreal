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

void FAndroidSentryScope::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagValueMethod, *GetJString(key), *GetJString(value));
}

FString FAndroidSentryScope::GetTag(const FString& key) const
{
	TMap<FString, FString> tags = GetTags();
	FString* tagValue = tags.Find(key);

	if (!tagValue)
		return FString();

	return *tagValue;
}

bool FAndroidSentryScope::TryGetTag(const FString& key, FString& value) const
{
	TMap<FString, FString> tags = GetTags();
	FString* tagValue = tags.Find(key);

	if (!tagValue)
		return false;

	value = *tagValue;

	return true;
}

void FAndroidSentryScope::RemoveTag(const FString& key)
{
	CallMethod<void>(RemoveTagMethod, *GetJString(key));
}

void FAndroidSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	for (const auto& tag : tags)
	{
		SetTag(tag.Key, tag.Value);
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

void FAndroidSentryScope::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	CallMethod<void>(SetContextMethod, *GetJString(key), FAndroidSentryConverters::VariantMapToNative(values)->GetJObject());
}

TMap<FString, FSentryVariant> FAndroidSentryScope::GetContext(const FString& key) const
{
	auto context = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getScopeContext", "(Lio/sentry/IScope;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key));
	return FAndroidSentryConverters::VariantMapToUnreal(*context);
}

bool FAndroidSentryScope::TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const
{
	auto context = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getScopeContext", "(Lio/sentry/IScope;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key));

	if (!context)
	{
		return false;
	}

	const FSentryVariant& contextVariant = FAndroidSentryConverters::VariantToUnreal(*context);
	if (contextVariant.GetType() == ESentryVariantType::Empty)
	{
		return false;
	}

	value = contextVariant.GetValue<TMap<FString, FSentryVariant>>();

	return true;
}

void FAndroidSentryScope::RemoveContext(const FString& key)
{
	CallMethod<void>(RemoveContextMethod, *GetJString(key));
}

void FAndroidSentryScope::SetExtra(const FString& key, const FSentryVariant& value)
{
	// Sentry's Android SDK currently supports only string type for extras (see https://github.com/getsentry/sentry-java/issues/2032)
	// Variants with array/map values will be set as strings
	// When retrieving such values using `GetExtra`, `TryGetExtra` or `GetExtras` they have to be interpreted as strings
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setScopeExtra", "(Lio/sentry/IScope;Ljava/lang/String;Ljava/lang/Object;)V",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key), FAndroidSentryConverters::VariantToNative(value)->GetJObject());
}

FSentryVariant FAndroidSentryScope::GetExtra(const FString& key) const
{
	TMap<FString, FSentryVariant> extras = GetExtras();
	FSentryVariant* extraValue = extras.Find(key);

	if (!extraValue)
		return FSentryVariant();

	return *extraValue;
}

bool FAndroidSentryScope::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	TMap<FString, FSentryVariant> extras = GetExtras();
	FSentryVariant* extraValue = extras.Find(key);

	if (!extraValue)
		return false;

	value = *extraValue;
	return true;
}

void FAndroidSentryScope::RemoveExtra(const FString& key)
{
	CallMethod<void>(RemoveExtraMethod, *GetJString(key));
}

void FAndroidSentryScope::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	for (const auto& extra : extras)
	{
		SetExtra(extra.Key, extra.Value);
	}
}

TMap<FString, FSentryVariant> FAndroidSentryScope::GetExtras() const
{
	auto extras = CallObjectMethod<jobject>(GetExtrasMethod);
	return FAndroidSentryConverters::VariantMapToUnreal(*extras);
}

void FAndroidSentryScope::Clear()
{
	CallMethod<void>(ClearMethod);
}