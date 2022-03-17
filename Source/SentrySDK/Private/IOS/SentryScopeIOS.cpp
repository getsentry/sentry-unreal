// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeIOS.h"

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

void SentryScopeIOS::RemoveTag(const FString& key)
{
	[ScopeIOS removeTagForKey:key.GetNSString()];
}

void SentryScopeIOS::SetTags(const TMap<FString, FString>& tags)
{
	[ScopeIOS setTags:SentryConvertorsIOS::StringMapToNative(tags)];
}

void SentryScopeIOS::SetDist(const FString& dist)
{
	[ScopeIOS setDist:dist.GetNSString()];
}

void SentryScopeIOS::SetEnvironment(const FString& environment)
{
	[ScopeIOS setEnvironment:environment.GetNSString()];
}

void SentryScopeIOS::SetFingerprint(const TArray<FString>& fingerprint)
{
	[ScopeIOS setFingerprint:SentryConvertorsIOS::StringArrayToNative(fingerprint)];
}

void SentryScopeIOS::SetLevel(ESentryLevel level)
{
	[ScopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
}

void SentryScopeIOS::Clear()
{
	[ScopeIOS clear];
}
