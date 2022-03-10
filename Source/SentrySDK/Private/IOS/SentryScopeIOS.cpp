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
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:tags.Num()];
	for (auto it = tags.CreateConstIterator(); it; ++it)
	{
		[dict setValue:it.Value().GetNSString() forKey:it.Key().GetNSString()];
	}

	[_scopeIOS setTags:dict];
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
	NSMutableArray *arr = [NSMutableArray arrayWithCapacity:fingerprint.Num()];
	for (auto it = fingerprint.CreateConstIterator(); it; ++it)
	{
		[arr addObject:it->GetNSString()];
	}

	[_scopeIOS setFingerprint:arr];
}

void SentryScopeIOS::SetLevel(ESentryLevel level)
{
	[_scopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
}

void SentryScopeIOS::Clear()
{
	[_scopeIOS clear];
}
