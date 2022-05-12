// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#import <Sentry/Sentry.h>

class USentryScope;
class USentryId;

class SentryConvertorsIOS
{
public:
	/** Conversions to native iOS types */
	static SentryLevel SentryLevelToNative(ESentryLevel level);
	static NSDictionary* StringMapToNative(const TMap<FString, FString>& map);
	static NSArray* StringArrayToNative(const TArray<FString>& array);
	static NSData* ByteDataToNative(const TArray<uint8>& array);

	/** Conversions from native iOS types */
	static ESentryLevel SentryLevelToUnreal(SentryLevel level);
	static TMap<FString, FString> StringMapToUnreal(NSDictionary* dict);
	static TArray<FString> StringArrayToUnreal(NSArray* array);
	static TArray<uint8> ByteDataToUnreal(NSData* data);
	static USentryScope* SentryScopeToUnreal(SentryScope* scope);
	static USentryId* SentryIdToUnreal(SentryId* id);

	/** Other conversions */
	static SentryLevel StringToSentryLevel(NSString* string);
};
