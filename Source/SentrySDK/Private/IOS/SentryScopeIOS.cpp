// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeIOS.h"

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

void SentryScopeIOS::Clear()
{
	[ScopeIOS clear];
}
