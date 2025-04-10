// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "Logging/LogVerbosity.h"

#include "SentryDataTypes.h"

class SentryLogUtils
{
public:
	static void LogStackTrace(const TCHAR* Heading, const ELogVerbosity::Type LogVerbosity, int FramesToSkip);
	static ESentryLevel ConvertLogVerbosityToSentryLevel(const ELogVerbosity::Type LogVerbosity);
};
