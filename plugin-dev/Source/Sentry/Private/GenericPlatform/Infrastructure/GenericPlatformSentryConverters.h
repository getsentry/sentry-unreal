// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryVariant.h"

#include "GenericPlatform/Convenience/GenericPlatformSentryInclude.h"
#include "GenericPlatform/GenericPlatformStackWalk.h"

#if USE_SENTRY_NATIVE

class FJsonValue;

class FGenericPlatformSentryConverters
{
public:
	/** Conversions to native types */
	static sentry_level_e SentryLevelToNative(ESentryLevel level);
	static sentry_value_t StringMapToNative(const TMap<FString, FString>& map);
	static sentry_value_t StringArrayToNative(const TArray<FString>& array);
	static sentry_value_t VariantToNative(const FSentryVariant& variant);
	static sentry_value_t VariantArrayToNative(const TArray<FSentryVariant>& array);
	static sentry_value_t VariantMapToNative(const TMap<FString, FSentryVariant>& map);
	static sentry_value_t AddressToNative(uint64 address);
	static sentry_value_t CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack);

	/** Conversions from native types */
	static ESentryLevel SentryLevelToUnreal(sentry_value_t level);
	static ESentryLevel SentryLevelToUnreal(sentry_level_t level);
	static FSentryVariant VariantToUnreal(sentry_value_t variant);
	static TMap<FString, FSentryVariant> VariantMapToUnreal(sentry_value_t map);
	static TArray<FSentryVariant> VariantArrayToUnreal(sentry_value_t array);
	static TMap<FString, FString> StringMapToUnreal(sentry_value_t map);
	static TArray<FString> StringArrayToUnreal(sentry_value_t array);

	/** Other conversions */
	static FString SentryLevelToString(ESentryLevel level);
	static TArray<uint8> SentryEnvelopeToByteArray(sentry_envelope_t* envelope);
	static ELogVerbosity::Type SentryLevelToLogVerbosity(sentry_level_t level);
	static TSharedPtr<FJsonValue> VariantToJsonValue(const FSentryVariant& variant);
	static TSharedPtr<FJsonValue> VariantArrayToJsonValue(const TArray<FSentryVariant>& array);
	static TSharedPtr<FJsonValue> VariantMapToJsonValue(const TMap<FString, FSentryVariant>& map);
};

#endif
