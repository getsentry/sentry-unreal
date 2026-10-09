// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "CoreTypes.h"
#include "Logging/LogVerbosity.h"

#include "SentryDataTypes.h"

class SentryLogUtils
{
public:
	static void LogStackTrace(const TCHAR* Heading, const ELogVerbosity::Type LogVerbosity, int FramesToSkip);
	static ESentryLevel ConvertLogVerbosityToSentryLevel(const ELogVerbosity::Type LogVerbosity);
	static bool IsCallstackLine(const FString& Line);
};
