// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

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

	/** Other conversions */
	static FString SentryLevelToString(ESentryLevel level);
};
