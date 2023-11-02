// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryDataTypes.generated.h"

class USentrySettings;

UENUM(BlueprintType)
enum class ESentryLevel : uint8
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureSettingsDelegate, USentrySettings*, Settings);
