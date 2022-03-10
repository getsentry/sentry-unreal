// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryScopeIOS::Init()
{
	_scopeIOS = [[SentryScope alloc] init];
}

void SentryScopeIOS::InitWithNativeObject(SentryScope* scope)
{
	_scopeIOS = scope;
}

void SentryScopeIOS::SetTagValue(const FString& key, const FString& value)
{
	[_scopeIOS setTagValue:value.GetNSString() forKey:key.GetNSString()];
}

void SentryScopeIOS::RemoveTag(const FString& key)
{
	[_scopeIOS removeTagForKey:key.GetNSString()];
}

void SentryScopeIOS::SetTags(const TMap<FString, FString>& tags)
{
	[_scopeIOS setTags:SentryConvertorsIOS::StringMapToNative(tags)];
}

void SentryScopeIOS::SetDist(const FString& dist)
{
	[_scopeIOS setDist:dist.GetNSString()];
}

void SentryScopeIOS::SetEnvironment(const FString& environment)
{
	[_scopeIOS setEnvironment:environment.GetNSString()];
}

void SentryScopeIOS::SetFingerprint(const TArray<FString>& fingerprint)
{
	[_scopeIOS setFingerprint:SentryConvertorsIOS::StringArrayToNative(fingerprint)];
}

void SentryScopeIOS::SetLevel(ESentryLevel level)
{
	[_scopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
}

void SentryScopeIOS::Clear()
{
	[_scopeIOS clear];
}
