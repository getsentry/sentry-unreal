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

void FGenericPlatformSentryScope::SetTag(const FString& key, const FString& value)
{
	Tags.Add(key, value);
}

FString FGenericPlatformSentryScope::GetTag(const FString& key) const
{
	if (!Tags.Contains(key))
		return FString();

	return Tags[key];
}

bool FGenericPlatformSentryScope::TryGetTag(const FString& key, FString& value) const
{
	if (!Tags.Contains(key))
		return false;

	value = Tags[key];
	return true;
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

void FGenericPlatformSentryScope::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	Contexts.Add(key, values);
}

TMap<FString, FSentryVariant> FGenericPlatformSentryScope::GetContext(const FString& key) const
{
	if (!Contexts.Contains(key))
		return TMap<FString, FSentryVariant>();

	return Contexts[key];
}

bool FGenericPlatformSentryScope::TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const
{
	if (!Contexts.Contains(key))
		return false;

	value = Contexts[key];
	return true;
}

void FGenericPlatformSentryScope::RemoveContext(const FString& key)
{
	if (!Contexts.Contains(key))
		return;

	Contexts.Remove(key);
}

void FGenericPlatformSentryScope::SetExtra(const FString& key, const FSentryVariant& value)
{
	Extra.Add(key, value);
}

FSentryVariant FGenericPlatformSentryScope::GetExtra(const FString& key) const
{
	if (!Extra.Contains(key))
		return FSentryVariant();

	return Extra[key];
}

bool FGenericPlatformSentryScope::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	if (!Extra.Contains(key))
		return false;

	value = Extra[key];
	return true;
}

void FGenericPlatformSentryScope::RemoveExtra(const FString& key)
{
	if (!Extra.Contains(key))
		return;

	Extra.Remove(key);
}

void FGenericPlatformSentryScope::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	Extra.Append(extras);
}

TMap<FString, FSentryVariant> FGenericPlatformSentryScope::GetExtras() const
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
		sentry_scope_set_extra(scope, TCHAR_TO_UTF8(*ExtraItem.Key), FGenericPlatformSentryConverters::VariantToNative(ExtraItem.Value));
	}

	for (const auto& ContextsItem : Contexts)
	{
		sentry_scope_set_context(scope, TCHAR_TO_UTF8(*ContextsItem.Key), FGenericPlatformSentryConverters::VariantMapToNative(ContextsItem.Value));
	}

	sentry_scope_set_level(scope, FGenericPlatformSentryConverters::SentryLevelToNative(Level));
}

#endif
