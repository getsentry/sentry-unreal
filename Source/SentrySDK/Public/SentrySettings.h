// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SentrySettings.generated.h"

UCLASS(Config = Engine, defaultconfig)
class SENTRYSDK_API USentrySettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core", Meta = (DisplayName = "DSN", ToolTip = "The DSN (Data Source Name) tells the SDK where to send the events to. Get your DSN in the Sentry dashboard."))
	FString DsnUrl;
};
