// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

#if USE_SENTRY_NATIVE

class USentryId;
class USentryTransaction;
class USentrySpan;

class SentryConvertorsDesktop
{
public:
	/** Conversions to native desktop (Windows/Mac) types */
	static sentry_level_e SentryLevelToNative(ESentryLevel level);
	static sentry_value_t StringMapToNative(const TMap<FString, FString>& map);
	static sentry_value_t StringArrayToNative(const TArray<FString>& array );

	/** Conversions from native desktop (Windows/Mac) types */
	static ESentryLevel SentryLevelToUnreal(sentry_value_t level);
	static ESentryLevel SentryLevelToUnreal(sentry_level_t level);
	static USentryId* SentryIdToUnreal(sentry_uuid_t id);
	static USentryTransaction* SentryTransactionToUnreal(sentry_transaction_t* transaction);
	static USentrySpan* SentrySpanToUnreal(sentry_span_t* span);
	static TMap<FString, FString> StringMapToUnreal(sentry_value_t map);
	static TArray<FString> StringArrayToUnreal(sentry_value_t array);

	/** Other conversions */
	static FString SentryLevelToString(ESentryLevel level);
	static TArray<uint8> SentryEnvelopeToByteArray(sentry_envelope_t* envelope);
};

#endif
