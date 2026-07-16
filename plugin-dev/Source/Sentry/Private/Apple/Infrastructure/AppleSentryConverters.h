// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "SentryDataTypes.h"
#include "SentryVariant.h"

#include "Convenience/AppleSentryInclude.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"

struct FSentryVariant;

class FAppleSentryConverters
{
public:
	/** Conversions to native Mac/iOS types */
	static SentryObjCLevel SentryLevelToNative(ESentryLevel level);
	static SentryObjCLogLevel SentryLogLevelToNative(ESentryLevel level);
	static NSDictionary* StringMapToNative(const TMap<FString, FString>& map);
	static NSArray* StringArrayToNative(const TArray<FString>& array);
	static NSData* ByteDataToNative(const TArray<uint8>& array);
	static id VariantToNative(const FSentryVariant& variant);
	static NSArray* VariantArrayToNative(const TArray<FSentryVariant>& variantArray);
	static NSDictionary* VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap);
	static SentryObjCStacktrace* CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack);
	static SentryObjCAttribute* VariantToAttributeNative(const FSentryVariant& variant);
	static SentryObjCAttributeContent* VariantToAttributeContentNative(const FSentryVariant& variant);
	static NSDictionary<NSString*, SentryObjCAttributeContent*>* VariantMapToAttributeContentNative(const TMap<FString, FSentryVariant>& variantMap);

	/** Conversions from native Mac/iOS types */
	static ESentryLevel SentryLevelToUnreal(SentryObjCLevel level);
	static ESentryLevel SentryLogLevelToUnreal(SentryObjCLogLevel level);
	static TMap<FString, FString> StringMapToUnreal(NSDictionary* dict);
	static TArray<FString> StringArrayToUnreal(NSArray* array);
	static TArray<uint8> ByteDataToUnreal(NSData* data);
	static FSentryVariant VariantToUnreal(id variant);
	static TArray<FSentryVariant> VariantArrayToUnreal(NSArray* variantArray);
	static TMap<FString, FSentryVariant> VariantMapToUnreal(NSDictionary* variantMap);

	/** Other conversions */
	static SentryObjCLevel StringToSentryLevel(NSString* string);
	static NSString* SentryLevelToString(SentryObjCLevel level);
	static FSentryVariant SentryAttributeToVariant(SentryObjCAttribute* attribute);
	static FSentryVariant SentryAttributeContentToVariant(SentryObjCAttributeContent* content);
};

#endif // !USE_SENTRY_NATIVE
