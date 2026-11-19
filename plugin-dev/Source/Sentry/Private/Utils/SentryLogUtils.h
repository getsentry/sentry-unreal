// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Logging/LogVerbosity.h"

#include "SentryDataTypes.h"

struct FStructuredLoggingLevels;

class SentryLogUtils
{
public:
	static void LogStackTrace(const TCHAR* Heading, const ELogVerbosity::Type LogVerbosity, int FramesToSkip);
	static ESentryLevel ConvertLogVerbosityToSentryLevel(const ELogVerbosity::Type LogVerbosity);
	static ESentryLogLevel ConvertLogVerbosityToSentryLogLevel(const ELogVerbosity::Type LogVerbosity);
	static bool IsCallstackLine(const FString& Line);

	/** Copies the Verbose flag to the VeryVerbose one for configs written before VeryVerbose had its own flag. */
	static void MigrateStructuredLoggingLevels(FStructuredLoggingLevels& Levels, const FString& ConfigValue);
};
