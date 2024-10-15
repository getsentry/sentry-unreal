// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeApple.h"

#include "SentryBreadcrumbApple.h"
#include "SentryAttachmentApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryScopeApple::SentryScopeApple()
{
	ScopeApple = [[SENTRY_APPLE_CLASS(SentryScope) alloc] init];
}

SentryScopeApple::SentryScopeApple(SentryScope* scope)
{
	ScopeApple = scope;
}

SentryScopeApple::~SentryScopeApple()
{
	// Put custom destructor logic here if needed
}

SentryScope* SentryScopeApple::GetNativeObject()
{
	return ScopeApple;
}

void SentryScopeApple::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbApple>(breadcrumb);

	[ScopeApple addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentryScopeApple::ClearBreadcrumbs()
{
	[ScopeApple clearBreadcrumbs];
}

void SentryScopeApple::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<SentryAttachmentApple> attachmentIOS = StaticCastSharedPtr<SentryAttachmentApple>(attachment);

	[ScopeApple addAttachment:attachmentIOS->GetNativeObject()];
}

void SentryScopeApple::ClearAttachments()
{
	[ScopeApple clearAttachments];
}

void SentryScopeApple::SetTagValue(const FString& key, const FString& value)
{
	[ScopeApple setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

FString SentryScopeApple::GetTagValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* tags = scopeDict[@"tags"];
	return FString(tags[key.GetNSString()]);
}

void SentryScopeApple::RemoveTag(const FString& key)
{
	[ScopeApple removeTagForKey:key.GetNSString()];
}

void SentryScopeApple::SetTags(const TMap<FString, FString>& tags)
{
	[ScopeApple setTags:SentryConvertorsApple::StringMapToNative(tags)];
}

TMap<FString, FString> SentryScopeApple::GetTags() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return SentryConvertorsApple::StringMapToUnreal(scopeDict[@"tags"]);
}

void SentryScopeApple::SetDist(const FString& dist)
{
	[ScopeApple setDist:dist.GetNSString()];
}

FString SentryScopeApple::GetDist() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return FString(scopeDict[@"dist"]);
}

void SentryScopeApple::SetEnvironment(const FString& environment)
{
	[ScopeApple setEnvironment:environment.GetNSString()];
}

FString SentryScopeApple::GetEnvironment() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return FString(scopeDict[@"environment"]);
}

void SentryScopeApple::SetFingerprint(const TArray<FString>& fingerprint)
{
	[ScopeApple setFingerprint:SentryConvertorsApple::StringArrayToNative(fingerprint)];
}

TArray<FString> SentryScopeApple::GetFingerprint() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSArray* fingerprint = scopeDict[@"fingerprint"];
	return SentryConvertorsApple::StringArrayToUnreal(fingerprint);
}

void SentryScopeApple::SetLevel(ESentryLevel level)
{
	[ScopeApple setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
}

ESentryLevel SentryScopeApple::GetLevel() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	SentryLevel level = SentryConvertorsApple::StringToSentryLevel(scopeDict[@"level"]);
	return SentryConvertorsApple::SentryLevelToUnreal(level);
}

void SentryScopeApple::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[ScopeApple setContextValue:SentryConvertorsApple::StringMapToNative(values) forKey:key.GetNSString()];
}

void SentryScopeApple::RemoveContext(const FString& key)
{
	[ScopeApple removeContextForKey:key.GetNSString()];
}

void SentryScopeApple::SetExtraValue(const FString& key, const FString& value)
{
	[ScopeApple setExtraValue:value.GetNSString() forKey:key.GetNSString()];
}

FString SentryScopeApple::GetExtraValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	NSDictionary* extras = scopeDict[@"extra"];
	return FString(extras[key.GetNSString()]);
}

void SentryScopeApple::RemoveExtra(const FString& key)
{
	[ScopeApple removeExtraForKey:key.GetNSString()];
}

void SentryScopeApple::SetExtras(const TMap<FString, FString>& extras)
{
	[ScopeApple setExtras:SentryConvertorsApple::StringMapToNative(extras)];
}

TMap<FString, FString> SentryScopeApple::GetExtras() const
{
	NSDictionary* scopeDict = [ScopeApple serialize];
	return SentryConvertorsApple::StringMapToUnreal(scopeDict[@"extra"]);
}

void SentryScopeApple::Clear()
{
	[ScopeApple clear];
}
