// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

#if USE_SENTRY_NATIVE

class USentryId;

class SentryConvertorsDesktop
{
public:
	/** Conversions to native desktop (Windows/Mac) types */
	static sentry_level_e SentryLevelToNative(ESentryLevel level);
	static sentry_value_t StringMapToNative(const TMap<FString, FString>& map);

	/** Conversions from native desktop (Windows/Mac) types */
	static ESentryLevel SentryLevelToUnreal(sentry_value_t level);
	static USentryId* SentryIdToUnreal(sentry_uuid_t id);
	static TMap<FString, FString> StringMapToUnreal(sentry_value_t map);

	/** Other conversions */
	static FString SentryLevelToString(ESentryLevel level);
	static TArray<uint8> SentryEnvelopeToByteArray(sentry_envelope_t* envelope);
};

#endif
