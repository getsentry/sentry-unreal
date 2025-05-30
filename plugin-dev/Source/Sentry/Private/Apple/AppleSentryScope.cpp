// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryScope.h"

#include "AppleSentryAttachment.h"
#include "AppleSentryBreadcrumb.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

FAppleSentryScope::FAppleSentryScope()
{
	ScopeApple = [[SentryScope alloc] init];
}

FAppleSentryScope::FAppleSentryScope(SentryScope* scope)
{
	ScopeApple = scope;
}

FAppleSentryScope::~FAppleSentryScope()
{
	// Put custom destructor logic here if needed
}

SentryScope* FAppleSentryScope::GetNativeObject()
{
	return ScopeApple;
}

void FAppleSentryScope::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<FAppleSentryBreadcrumb> breadcrumbIOS = StaticCastSharedPtr<FAppleSentryBreadcrumb>(breadcrumb);

	[ScopeApple addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentryScope::ClearBreadcrumbs()
{
	[ScopeApple clearBreadcrumbs];
}

void FAppleSentryScope::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAppleSentryAttachment> attachmentIOS = StaticCastSharedPtr<FAppleSentryAttachment>(attachment);

	[ScopeApple addAttachment:attachmentIOS->GetNativeObject()];
}

void FAppleSentryScope::ClearAttachments()
{
	[ScopeApple clearAttachments];
}

void FAppleSentryScope::SetTagValue(const FString& key, const FString& value)
{
	[ScopeApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

FString FAppleSentryScope::GetTagValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* tags = scopeDict[@"tags"];
	return FString(tags[key.GetNSString()]);
}

void FAppleSentryScope::RemoveTag(const FString& key)
{
	[ScopeApple removeTagForKey:key.GetNSString()];
}

void FAppleSentryScope::SetTags(const TMap<FString, FString>& tags)
{
	[ScopeApple setTags:FAppleSentryConverters::StringMapToNative(tags)];
}

TMap<FString, FString> FAppleSentryScope::GetTags() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return FAppleSentryConverters::StringMapToUnreal(scopeDict[@"tags"]);
}

void FAppleSentryScope::SetFingerprint(const TArray<FString>& fingerprint)
{
	[ScopeApple setFingerprint:FAppleSentryConverters::StringArrayToNative(fingerprint)];
}

TArray<FString> FAppleSentryScope::GetFingerprint() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSArray* fingerprint = scopeDict[@"fingerprint"];
	return FAppleSentryConverters::StringArrayToUnreal(fingerprint);
}

void FAppleSentryScope::SetLevel(ESentryLevel level)
{
	[ScopeApple setLevel:FAppleSentryConverters::SentryLevelToNative(level)];
}

ESentryLevel FAppleSentryScope::GetLevel() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	SentryLevel level = FAppleSentryConverters::StringToSentryLevel(scopeDict[@"level"]);
	return FAppleSentryConverters::SentryLevelToUnreal(level);
}

void FAppleSentryScope::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[ScopeApple setContextValue:FAppleSentryConverters::StringMapToNative(values) forKey:key.GetNSString()];
}

void FAppleSentryScope::RemoveContext(const FString& key)
{
	[ScopeApple removeContextForKey:key.GetNSString()];
}

void FAppleSentryScope::SetExtraValue(const FString& key, const FString& value)
{
	[ScopeApple setExtraValue:value.GetNSString() forKey:key.GetNSString()];
}

FString FAppleSentryScope::GetExtraValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* extras = scopeDict[@"extra"];
	return FString(extras[key.GetNSString()]);
}

void FAppleSentryScope::RemoveExtra(const FString& key)
{
	[ScopeApple removeExtraForKey:key.GetNSString()];
}

void FAppleSentryScope::SetExtras(const TMap<FString, FString>& extras)
{
	[ScopeApple setExtras:FAppleSentryConverters::StringMapToNative(extras)];
}

TMap<FString, FString> FAppleSentryScope::GetExtras() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return FAppleSentryConverters::StringMapToUnreal(scopeDict[@"extra"]);
}

void FAppleSentryScope::Clear()
{
	[ScopeApple clear];
}
