// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeLogHandler.h"

#include "CppBeforeLogHandler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppBeforeLogHandler : public USentryBeforeLogHandler
{
	GENERATED_BODY()

public:
	virtual USentryLog* HandleBeforeLog_Implementation(USentryLog* LogData) override;
};
