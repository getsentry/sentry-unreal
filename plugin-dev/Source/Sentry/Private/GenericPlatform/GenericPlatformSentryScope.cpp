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
{
	Scope = sentry_local_scope_new();
}

FGenericPlatformSentryScope::FGenericPlatformSentryScope(sentry_scope_t* scope)
	: Scope(scope)
{
}

FGenericPlatformSentryScope::~FGenericPlatformSentryScope()
{
}

void FGenericPlatformSentryScope::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	sentry_scope_add_breadcrumb(Scope, StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(breadcrumb)->GetNativeObject());
}

void FGenericPlatformSentryScope::ClearBreadcrumbs()
{
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
	sentry_scope_set_tag(Scope, TCHAR_TO_UTF8(*key), TCHAR_TO_UTF8(*value));
}

FString FGenericPlatformSentryScope::GetTagValue(const FString& key) const
{
	return FString();
}

void FGenericPlatformSentryScope::RemoveTag(const FString& key)
{
}

void FGenericPlatformSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	for (const auto& tagItem : tags)
	{
		SetTagValue(tagItem.Key, tagItem.Value);
	}
}

TMap<FString, FString> FGenericPlatformSentryScope::GetTags() const
{
	return TMap<FString, FString>();
}

void FGenericPlatformSentryScope::SetDist(const FString& dist)
{
	// Set via options during initialization
}

FString FGenericPlatformSentryScope::GetDist() const
{
	return FString();
}

void FGenericPlatformSentryScope::SetEnvironment(const FString& environment)
{
	// Set via options during initialization
}

FString FGenericPlatformSentryScope::GetEnvironment() const
{
	return FString();
}

void FGenericPlatformSentryScope::SetFingerprint(const TArray<FString>& fingerprint)
{
	sentry_scope_set_fingerprints(Scope, FGenericPlatformSentryConverters::StringArrayToNative(fingerprint));
}

TArray<FString> FGenericPlatformSentryScope::GetFingerprint() const
{
	return TArray<FString>();
}

void FGenericPlatformSentryScope::SetLevel(ESentryLevel level)
{
	sentry_scope_set_level(Scope, FGenericPlatformSentryConverters::SentryLevelToNative(level));
}

ESentryLevel FGenericPlatformSentryScope::GetLevel() const
{
	return ESentryLevel::Debug;
}

void FGenericPlatformSentryScope::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	sentry_scope_set_context(Scope, TCHAR_TO_UTF8(*key), FGenericPlatformSentryConverters::StringMapToNative(values));
}

void FGenericPlatformSentryScope::RemoveContext(const FString& key)
{
}

void FGenericPlatformSentryScope::SetExtraValue(const FString& key, const FString& value)
{
	sentry_scope_set_extra(Scope, TCHAR_TO_UTF8(*key), sentry_value_new_string(TCHAR_TO_UTF8(*value)));
}

FString FGenericPlatformSentryScope::GetExtraValue(const FString& key) const
{
	return FString();
}

void FGenericPlatformSentryScope::RemoveExtra(const FString& key)
{
}

void FGenericPlatformSentryScope::SetExtras(const TMap<FString, FString>& extras)
{
	for (const auto& extraItem : extras)
	{
		SetExtraValue(extraItem.Key, extraItem.Value);
	}
}

TMap<FString, FString> FGenericPlatformSentryScope::GetExtras() const
{
	return TMap<FString, FString>();
}

void FGenericPlatformSentryScope::Clear()
{
}

#endif
