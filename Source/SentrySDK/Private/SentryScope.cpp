// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScope.h"

#if PLATFORM_IOS
#import <Sentry/Sentry.h>
#include "IOS/SentryConvertorsIOS.h"
#endif

void USentryScope::SetTagValue(const FString& Key, const FString& Value)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::RemoveTag(const FString& Key)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::SetTags(const TMap<FString, FString>& Tags)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::SetDist(const FString& Dist)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::SetEnvironment(const FString& Environment)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::SetFingerprint(const TArray<FString>& Fingerprint)
{
#if PLATFORM_IOS
	// TODO
#endif
}

void USentryScope::SetLevel(ESentryLevel Level)
{
#if PLATFORM_IOS
	[_scopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(Level)];
#endif
}

void USentryScope::Clear()
{
#if PLATFORM_IOS
	// TODO
#endif
}

#if PLATFORM_IOS
void USentryScope::InitWithNativeObjectIOS(SentryScope* scope)
{
	_scopeIOS = scope;
}
#endif
