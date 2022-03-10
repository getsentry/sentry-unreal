// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryScopeIOS::InitWithNativeObject(SentryScope* scope)
{
	_scopeIOS = scope;
}

void SentryScopeIOS::SetTagValue(const FString& Key, const FString& Value)
{
}

void SentryScopeIOS::RemoveTag(const FString& Key)
{
}

void SentryScopeIOS::SetTags(const TMap<FString, FString>& Tags)
{
}

void SentryScopeIOS::SetDist(const FString& Dist)
{
}

void SentryScopeIOS::SetEnvironment(const FString& Environment)
{
}

void SentryScopeIOS::SetFingerprint(const TArray<FString>& Fingerprint)
{	
}

void SentryScopeIOS::SetLevel(ESentryLevel Level)
{
	[_scopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(Level)];
}

void SentryScopeIOS::Clear()
{
}
