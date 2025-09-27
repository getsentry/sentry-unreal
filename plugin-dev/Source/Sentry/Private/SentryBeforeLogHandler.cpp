// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeLogHandler.h"
#include "SentryLog.h"

USentryLog* USentryBeforeLogHandler::HandleBeforeLog_Implementation(USentryLog* LogData)
{
	return LogData;
}