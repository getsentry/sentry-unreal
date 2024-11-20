// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

#include "GenericPlatform/GenericPlatformStackWalk.h"

#if USE_SENTRY_NATIVE

class SentryConvertorsDesktop
{
public:
	/** Conversions to native desktop (Windows/Mac) types */
	static sentry_level_e SentryLevelToNative(ESentryLevel level);
	static sentry_value_t StringMapToNative(const TMap<FString, FString>& map);
	static sentry_value_t StringArrayToNative(const TArray<FString>& array);
	static sentry_value_t AddressToNative(uint64 address);
	static sentry_value_t CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack);

	/** Conversions from native desktop (Windows/Mac) types */
	static ESentryLevel SentryLevelToUnreal(sentry_value_t level);
	static ESentryLevel SentryLevelToUnreal(sentry_level_t level);
	static TMap<FString, FString> StringMapToUnreal(sentry_value_t map);
	static TArray<FString> StringArrayToUnreal(sentry_value_t array);

	/** Other conversions */
	static FString SentryLevelToString(ESentryLevel level);
	static TArray<uint8> SentryEnvelopeToByteArray(sentry_envelope_t* envelope);
	static ELogVerbosity::Type SentryLevelToLogVerbosity(sentry_level_t level);

	static void SentryCrashContextToString(const sentry_ucontext_t* crashContext, TCHAR* outErrorString, int32 errorStringBufSize);
};

#endif
