// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryDataTypes.h"

#include "SentryBeforeLogHandler.generated.h"

class USentryLog;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeLogHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryLog* HandleBeforeLog(USentryLog* LogData);
	virtual USentryLog* HandleBeforeLog_Implementation(USentryLog* LogData);
};