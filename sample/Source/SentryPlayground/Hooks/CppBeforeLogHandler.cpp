// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppBeforeLogHandler.h"

#include "SentryLog.h"

USentryLog* UCppBeforeLogHandler::HandleBeforeLog_Implementation(USentryLog* LogData)
{
	LogData->SetAttribute(TEXT("handler_added"), FSentryVariant(TEXT("added_value")));
	LogData->RemoveAttribute(TEXT("to_be_removed"));

	FSentryVariant LevelOverride;
	if (LogData->TryGetAttribute(TEXT("level_override"), LevelOverride))
	{
		const UEnum* LevelEnum = StaticEnum<ESentryLogLevel>();

		LogData->SetAttribute(TEXT("level_read"), FSentryVariant(LevelEnum->GetNameStringByValue(static_cast<int64>(LogData->GetLevel()))));
		LogData->SetLevel(static_cast<ESentryLogLevel>(LevelEnum->GetValueByNameString(LevelOverride.GetValue<FString>())));
	}

	return Super::HandleBeforeLog_Implementation(LogData);
}
