// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryVariant.h"

#include "Convenience/AppleSentryInclude.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"

struct FSentryVariant;

class FAppleSentryConverters
{
public:
	/** Conversions to native Mac/iOS types */
	static SentryLevel SentryLevelToNative(ESentryLevel level);
	static NSDictionary* StringMapToNative(const TMap<FString, FString>& map);
	static NSArray* StringArrayToNative(const TArray<FString>& array);
	static NSData* ByteDataToNative(const TArray<uint8>& array);
	static id VariantToNative(const FSentryVariant& variant);
	static NSArray* VariantArrayToNative(const TArray<FSentryVariant>& variantArray);
	static NSDictionary* VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap);
	static SentryStacktrace* CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack);

	/** Conversions from native Mac/iOS types */
	static ESentryLevel SentryLevelToUnreal(SentryLevel level);
	static TMap<FString, FString> StringMapToUnreal(NSDictionary* dict);
	static TArray<FString> StringArrayToUnreal(NSArray* array);
	static TArray<uint8> ByteDataToUnreal(NSData* data);
	static FSentryVariant VariantToUnreal(id variant);
	static TArray<FSentryVariant> VariantArrayToUnreal(NSArray* variantArray);
	static TMap<FString, FSentryVariant> VariantMapToUnreal(NSDictionary* variantMap);

	/** Other conversions */
	static SentryLevel StringToSentryLevel(NSString* string);
};
