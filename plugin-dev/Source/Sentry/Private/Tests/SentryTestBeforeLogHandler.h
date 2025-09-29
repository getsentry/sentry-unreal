// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryBeforeLogHandler.h"
#include "SentryLog.h"

#include "SentryTestBeforeLogHandler.generated.h"

UCLASS()
class UTestBeforeLogHandler : public USentryBeforeLogHandler
{
	GENERATED_BODY()
public:
	virtual USentryLog* HandleBeforeLog_Implementation(USentryLog* LogData) override
	{
		OnTestBeforeLogHandler.ExecuteIfBound(LogData);
		return Super::HandleBeforeLog_Implementation(LogData);
	}

	static TDelegate<void(USentryLog*)> OnTestBeforeLogHandler;
};