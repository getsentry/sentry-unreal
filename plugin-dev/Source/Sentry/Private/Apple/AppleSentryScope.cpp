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

void FAppleSentryScope::SetTag(const FString& key, const FString& value)
{
	[ScopeApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

FString FAppleSentryScope::GetTag(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* tags = scopeDict[@"tags"];
	return FString(tags[key.GetNSString()]);
}

bool FAppleSentryScope::TryGetTag(const FString& key, FString& value) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* tags = scopeDict[@"tags"];

	NSString* tag = [tags objectForKey:key.GetNSString()];

	if (!tag)
	{
		return false;
	}

	value = FString(tag);
	return true;
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

void FAppleSentryScope::SetExtra(const FString& key, const FSentryVariant& value)
{
	[ScopeApple setExtraValue:FAppleSentryConverters::VariantToNative(value) forKey:key.GetNSString()];
}

FSentryVariant FAppleSentryScope::GetExtra(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* extras = scopeDict[@"extra"];

	id extra = [extras objectForKey:key.GetNSString()];

	if (!extra)
	{
		return FSentryVariant();
	}

	return FAppleSentryConverters::VariantToUnreal(extra);
}

bool FAppleSentryScope::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* extras = scopeDict[@"extra"];

	id extra = [extras objectForKey:key.GetNSString()];

	if (!extra)
	{
		return false;
	}

	value = FAppleSentryConverters::VariantToUnreal(extra);
	return true;
}

void FAppleSentryScope::RemoveExtra(const FString& key)
{
	[ScopeApple removeExtraForKey:key.GetNSString()];
}

void FAppleSentryScope::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	[ScopeApple setExtras:FAppleSentryConverters::VariantMapToNative(extras)];
}

TMap<FString, FSentryVariant> FAppleSentryScope::GetExtras() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return FAppleSentryConverters::VariantMapToUnreal(scopeDict[@"extra"]);
}

void FAppleSentryScope::Clear()
{
	[ScopeApple clear];
}
