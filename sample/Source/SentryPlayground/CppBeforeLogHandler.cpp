// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppBeforeLogHandler.h"

#include "SentryLog.h"

USentryLog* UCppBeforeLogHandler::HandleBeforeLog_Implementation(USentryLog* LogData)
{
	LogData->SetAttribute(TEXT("handler_added"), FSentryVariant(TEXT("added_value")));
	LogData->RemoveAttribute(TEXT("to_be_removed"));

	return Super::HandleBeforeLog_Implementation(LogData);
}
