// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.generated.h"

UENUM(BlueprintType)
enum class ESentryLevel : uint8
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

UENUM(BlueprintType)
enum class ESentryCrashedLastRun : uint8
{
	NotEvaluated,
	NotCrashed,
	Crashed
};