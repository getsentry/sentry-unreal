// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryScope.h"
#include "GenericPlatformSentryBreadcrumb.h"
#include "GenericPlatformSentryEvent.h"

#include "Interface/SentryAttachmentInterface.h"

#include "SentryModule.h"
#include "SentrySettings.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryScope::FGenericPlatformSentryScope()
	: Level(ESentryLevel::Debug)
{
}

FGenericPlatformSentryScope::~FGenericPlatformSentryScope()
{
}

void FGenericPlatformSentryScope::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	if (Breadcrumbs.Num() >= FSentryModule::Get().GetSettings()->MaxBreadcrumbs)
	{
		Breadcrumbs.PopFront();
	}

	Breadcrumbs.Add(StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(breadcrumb));
}

void FGenericPlatformSentryScope::ClearBreadcrumbs()
{
	Breadcrumbs.Empty();
}

void FGenericPlatformSentryScope::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	// Not available for generic platform
}

void FGenericPlatformSentryScope::ClearAttachments()
{
	// Not available for generic platform
}

void FGenericPlatformSentryScope::SetTagValue(const FString& key, const FString& value)
{
	Tags.Add(key, value);
}

FString FGenericPlatformSentryScope::GetTagValue(const FString& key) const
{
	if (!Tags.Contains(key))
		return FString();

	return Tags[key];
}

void FGenericPlatformSentryScope::RemoveTag(const FString& key)
{
	Tags.Remove(key);
}

void FGenericPlatformSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	Tags.Append(tags);
}

TMap<FString, FString> FGenericPlatformSentryScope::GetTags() const
{
	return Tags;
}

void FGenericPlatformSentryScope::SetFingerprint(const TArray<FString>& fingerprint)
{
	Fingerprint = fingerprint;
}

TArray<FString> FGenericPlatformSentryScope::GetFingerprint() const
{
	return Fingerprint;
}

void FGenericPlatformSentryScope::SetLevel(ESentryLevel level)
{
	Level = level;
}

ESentryLevel FGenericPlatformSentryScope::GetLevel() const
{
	return Level;
}

void FGenericPlatformSentryScope::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	Contexts.Add(key, values);
}

void FGenericPlatformSentryScope::RemoveContext(const FString& key)
{
	if (!Contexts.Contains(key))
		return;

	Contexts.Remove(key);
}

void FGenericPlatformSentryScope::SetExtraValue(const FString& key, const FString& value)
{
	Extra.Add(key, value);
}

FString FGenericPlatformSentryScope::GetExtraValue(const FString& key) const
{
	if (!Extra.Contains(key))
		return FString();

	return Extra[key];
}

void FGenericPlatformSentryScope::RemoveExtra(const FString& key)
{
	if (!Extra.Contains(key))
		return;

	Extra.Remove(key);
}

void FGenericPlatformSentryScope::SetExtras(const TMap<FString, FString>& extras)
{
	Extra.Append(extras);
}

TMap<FString, FString> FGenericPlatformSentryScope::GetExtras() const
{
	return Extra;
}

void FGenericPlatformSentryScope::Clear()
{
	Dist = FString();
	Environment = FString();
	Fingerprint.Empty();
	Tags.Empty();
	Extra.Empty();
	Contexts.Empty();
	Breadcrumbs.Empty();
	Level = ESentryLevel::Debug;
}

void FGenericPlatformSentryScope::Apply(sentry_scope_t* scope)
{

	for (const auto& Breadcrumb : Breadcrumbs)
	{
		sentry_value_t nativeBreadcrumb = Breadcrumb->GetNativeObject();
		sentry_scope_add_breadcrumb(scope, nativeBreadcrumb);
	}

	if (Fingerprint.Num() > 0)
	{
		sentry_scope_set_fingerprints(scope, FGenericPlatformSentryConverters::StringArrayToNative(Fingerprint));
	}

	for (const auto& TagItem : Tags)
	{
		sentry_scope_set_tag(scope, TCHAR_TO_UTF8(*TagItem.Key), TCHAR_TO_UTF8(*TagItem.Value));
	}

	for (const auto& ExtraItem : Extra)
	{
		sentry_scope_set_extra(scope, TCHAR_TO_UTF8(*ExtraItem.Key), sentry_value_new_string(TCHAR_TO_UTF8(*ExtraItem.Value)));
	}

	for (const auto& ContextsItem : Contexts)
	{
		sentry_scope_set_context(scope, TCHAR_TO_UTF8(*ContextsItem.Key), FGenericPlatformSentryConverters::StringMapToNative(ContextsItem.Value));
	}

	sentry_scope_set_level(scope, FGenericPlatformSentryConverters::SentryLevelToNative(Level));
}

#endif
