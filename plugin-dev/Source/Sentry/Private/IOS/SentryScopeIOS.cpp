// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeIOS.h"

#include "SentryBreadcrumbIOS.h"
#include "SentryAttachmentIOS.h"

#include "SentryBreadcrumb.h"
#include "SentryAttachment.h"

#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryScopeIOS::SentryScopeIOS()
{
	ScopeIOS = [[SentryScope alloc] init];
}

SentryScopeIOS::SentryScopeIOS(SentryScope* scope)
{
	ScopeIOS = scope;
}

SentryScopeIOS::~SentryScopeIOS()
{
	// Put custom destructor logic here if needed
}

SentryScope* SentryScopeIOS::GetNativeObject()
{
	return ScopeIOS;
}

void SentryScopeIOS::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbIOS> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbIOS>(breadcrumb->GetNativeImpl());

	[ScopeIOS addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentryScopeIOS::ClearBreadcrumbs()
{
	[ScopeIOS clearBreadcrumbs];
}

void SentryScopeIOS::AddAttachment(USentryAttachment* attachment)
{
	TSharedPtr<SentryAttachmentIOS> attachmentIOS = StaticCastSharedPtr<SentryAttachmentIOS>(attachment->GetNativeImpl());

	[ScopeIOS addAttachment:attachmentIOS->GetNativeObject()];
}

void SentryScopeIOS::ClearAttachments()
{
	[ScopeIOS clearAttachments];
}

void SentryScopeIOS::SetTagValue(const FString& key, const FString& value)
{
	[ScopeIOS setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

FString SentryScopeIOS::GetTagValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	NSDictionary* tags = scopeDict[@"tags"];
	return FString(tags[key.GetNSString()]);
}

void SentryScopeIOS::RemoveTag(const FString& key)
{
	[ScopeIOS removeTagForKey:key.GetNSString()];
}

void SentryScopeIOS::SetTags(const TMap<FString, FString>& tags)
{
	[ScopeIOS setTags:SentryConvertorsIOS::StringMapToNative(tags)];
}

TMap<FString, FString> SentryScopeIOS::GetTags() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	return SentryConvertorsIOS::StringMapToUnreal(scopeDict[@"tags"]);
}

void SentryScopeIOS::SetDist(const FString& dist)
{
	[ScopeIOS setDist:dist.GetNSString()];
}

FString SentryScopeIOS::GetDist() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	return FString(scopeDict[@"dist"]);
}

void SentryScopeIOS::SetEnvironment(const FString& environment)
{
	[ScopeIOS setEnvironment:environment.GetNSString()];
}

FString SentryScopeIOS::GetEnvironment() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	return FString(scopeDict[@"environment"]);
}

void SentryScopeIOS::SetFingerprint(const TArray<FString>& fingerprint)
{
	[ScopeIOS setFingerprint:SentryConvertorsIOS::StringArrayToNative(fingerprint)];
}

TArray<FString> SentryScopeIOS::GetFingerprint() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	NSArray* fingerprint = scopeDict[@"fingerprint"];
	return SentryConvertorsIOS::StringArrayToUnreal(fingerprint);
}

void SentryScopeIOS::SetLevel(ESentryLevel level)
{
	[ScopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
}

ESentryLevel SentryScopeIOS::GetLevel() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	SentryLevel level = SentryConvertorsIOS::StringToSentryLevel(scopeDict[@"level"]);
	return SentryConvertorsIOS::SentryLevelToUnreal(level);
}

void SentryScopeIOS::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[ScopeIOS setContextValue:SentryConvertorsIOS::StringMapToNative(values) forKey:key.GetNSString()];
}

void SentryScopeIOS::RemoveContext(const FString& key)
{
	[ScopeIOS removeContextForKey:key.GetNSString()];
}

void SentryScopeIOS::SetExtraValue(const FString& key, const FString& value)
{
	[ScopeIOS setExtraValue:value.GetNSString() forKey:key.GetNSString()];
}

FString SentryScopeIOS::GetExtraValue(const FString& key) const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	NSDictionary* extras = scopeDict[@"extra"];
	return FString(extras[key.GetNSString()]);
}

void SentryScopeIOS::RemoveExtra(const FString& key)
{
	[ScopeIOS removeExtraForKey:key.GetNSString()];
}

void SentryScopeIOS::SetExtras(const TMap<FString, FString>& extras)
{
	[ScopeIOS setExtras:SentryConvertorsIOS::StringMapToNative(extras)];
}

TMap<FString, FString> SentryScopeIOS::GetExtras() const
{
	NSDictionary* scopeDict = [ScopeIOS serialize];
	return SentryConvertorsIOS::StringMapToUnreal(scopeDict[@"extra"]);
}

void SentryScopeIOS::Clear()
{
	[ScopeIOS clear];
}
