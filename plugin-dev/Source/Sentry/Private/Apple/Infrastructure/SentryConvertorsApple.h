// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"

class SentryConvertorsApple
{
public:
	/** Conversions to native iOS types */
	static SentryLevel SentryLevelToNative(ESentryLevel level);
	static NSDictionary* StringMapToNative(const TMap<FString, FString>& map);
	static NSArray* StringArrayToNative(const TArray<FString>& array);
	static NSData* ByteDataToNative(const TArray<uint8>& array);
	static SentryStacktrace* CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack);

	/** Conversions from native iOS types */
	static ESentryLevel SentryLevelToUnreal(SentryLevel level);
	static TMap<FString, FString> StringMapToUnreal(NSDictionary* dict);
	static TArray<FString> StringArrayToUnreal(NSArray* array);
	static TArray<uint8> ByteDataToUnreal(NSData* data);

	/** Other conversions */
	static SentryLevel StringToSentryLevel(NSString* string);
};
