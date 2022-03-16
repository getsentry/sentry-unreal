// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsIOS.h"
#include "SentryScope.h"
#include "SentryScopeIOS.h"

SentryLevel SentryConvertorsIOS::SentryLevelToNative(ESentryLevel level)
{
	SentryLevel nativeLevel;

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = kSentryLevelDebug;
		break;
	case ESentryLevel::Info:
		nativeLevel = kSentryLevelInfo;
		break;
	case ESentryLevel::Warning:
		nativeLevel = kSentryLevelWarning;
		break;
	case ESentryLevel::Error:
		nativeLevel = kSentryLevelError;
		break;
	case ESentryLevel::Fatal:
		nativeLevel = kSentryLevelFatal;
		break;
	default:
		NSLog(@"Unknown sentry level value used!");
	}

	return nativeLevel;
}

NSDictionary* SentryConvertorsIOS::StringMapToNative(const TMap<FString, FString>& map)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:map.Num()];

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		[dict setValue:it.Value().GetNSString() forKey:it.Key().GetNSString()];
	}

	return dict;
}

NSArray* SentryConvertorsIOS::StringArrayToNative(const TArray<FString>& array)
{
	NSMutableArray *arr = [NSMutableArray arrayWithCapacity:array.Num()];

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		[arr addObject:it->GetNSString()];
	}

	return arr;
}

USentryScope* SentryConvertorsIOS::SentryScopeToUnreal(TSharedPtr<SentryScopeIOS> scope)
{
	USentryScope* unrealScope = NewObject<USentryScope>();
	unrealScope->InitWithNativeImplIOS(scope);
	return unrealScope;
}