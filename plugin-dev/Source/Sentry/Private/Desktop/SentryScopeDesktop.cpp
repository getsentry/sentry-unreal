// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryScopeDesktop.h"
#include "SentryBreadcrumbDesktop.h"
#include "SentryEventDesktop.h"

#include "SentryBreadcrumb.h"
#include "SentryAttachment.h"
#include "SentryEvent.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentryScopeDesktop::SentryScopeDesktop()
	: LevelDesktop(ESentryLevel::Debug)
{
}

SentryScopeDesktop::~SentryScopeDesktop()
{
}

void SentryScopeDesktop::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	BreadcrumbsDesktop.Add(breadcrumb->GetNativeImpl());
}

void SentryScopeDesktop::ClearBreadcrumbs()
{
	BreadcrumbsDesktop.Empty();
}

void SentryScopeDesktop::AddAttachment(USentryAttachment* attachment)
{
	// Not available for desktop
}

void SentryScopeDesktop::ClearAttachments()
{
	// Not available for desktop
}

void SentryScopeDesktop::SetTagValue(const FString& key, const FString& value)
{
	TagsDesktop.Add(key, value);
}

FString SentryScopeDesktop::GetTagValue(const FString& key) const
{
	if(!TagsDesktop.Contains(key))
		return FString();

	return TagsDesktop[key];
}

void SentryScopeDesktop::RemoveTag(const FString& key)
{
	TagsDesktop.Remove(key);
}

void SentryScopeDesktop::SetTags(const TMap<FString, FString>& tags)
{
	TagsDesktop.Append(tags);
}

TMap<FString, FString> SentryScopeDesktop::GetTags() const
{
	return TagsDesktop;
}

void SentryScopeDesktop::SetDist(const FString& dist)
{
	Dist = dist;
}

FString SentryScopeDesktop::GetDist() const
{
	return Dist;
}

void SentryScopeDesktop::SetEnvironment(const FString& environment)
{
	Environment = environment;
}

FString SentryScopeDesktop::GetEnvironment() const
{
	return Environment;
}

void SentryScopeDesktop::SetFingerprint(const TArray<FString>& fingerprint)
{
	FingerprintDesktop = fingerprint;
}

TArray<FString> SentryScopeDesktop::GetFingerprint() const
{
	return FingerprintDesktop;
}

void SentryScopeDesktop::SetLevel(ESentryLevel level)
{
	LevelDesktop = level;
}

ESentryLevel SentryScopeDesktop::GetLevel() const
{

	return LevelDesktop;
}

void SentryScopeDesktop::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	ContextsDesktop.Add(key, values);
}

void SentryScopeDesktop::RemoveContext(const FString& key)
{
	if(!ContextsDesktop.Contains(key))
		return;

	ContextsDesktop.Remove(key);
}

void SentryScopeDesktop::SetExtraValue(const FString& key, const FString& value)
{
	ExtraDesktop.Add(key, value);
}

FString SentryScopeDesktop::GetExtraValue(const FString& key) const
{
	if(!ExtraDesktop.Contains(key))
		return FString();

	return ExtraDesktop[key];
}

void SentryScopeDesktop::RemoveExtra(const FString& key)
{
	if(!ExtraDesktop.Contains(key))
		return;

	ExtraDesktop.Remove(key);
}

void SentryScopeDesktop::SetExtras(const TMap<FString, FString>& extras)
{
	ExtraDesktop.Append(extras);
}

TMap<FString, FString> SentryScopeDesktop::GetExtras() const
{
	return ExtraDesktop;
}

void SentryScopeDesktop::Clear()
{
	Dist = FString();
	Environment = FString();
	FingerprintDesktop.Empty();
	TagsDesktop.Empty();
	ExtraDesktop.Empty();
	ContextsDesktop.Empty();
	BreadcrumbsDesktop.Empty();
	LevelDesktop = ESentryLevel::Debug;
}

void SentryScopeDesktop::Apply(USentryEvent* event)
{
	TSharedPtr<SentryEventDesktop> eventDesktop = StaticCastSharedPtr<SentryEventDesktop>(event->GetNativeImpl());

	sentry_value_t nativeEvent = eventDesktop->GetNativeObject();

	if(!Dist.IsEmpty())
		sentry_value_set_by_key(nativeEvent, "dist", sentry_value_new_string(TCHAR_TO_ANSI(*Dist)));
	if(!Environment.IsEmpty())
		sentry_value_set_by_key(nativeEvent, "environment", sentry_value_new_string(TCHAR_TO_ANSI(*Environment)));

	if(!FingerprintDesktop.IsEmpty())
		sentry_value_set_by_key(nativeEvent, "fingerprint", SentryConvertorsDesktop::StringArrayToNative(FingerprintDesktop));
}

#endif
