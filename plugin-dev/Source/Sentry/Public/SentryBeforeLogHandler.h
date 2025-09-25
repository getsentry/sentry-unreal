// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryDataTypes.h"

#include "SentryBeforeLogHandler.generated.h"

class USentryLogData;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeLogHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryLogData* HandleBeforeLog(USentryLogData* LogData);
	virtual USentryLogData* HandleBeforeLog_Implementation(USentryLogData* LogData);
};