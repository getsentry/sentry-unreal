// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeLogHandler.h"
#include "SentryLogData.h"

USentryLogData* USentryBeforeLogHandler::HandleBeforeLog_Implementation(USentryLogData* LogData)
{
	return LogData;
}