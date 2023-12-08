// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTransaction.generated.h"

/**
 * Representation of an activity to measure or track.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryTransaction : public UObject
{
	GENERATED_BODY()

public:
	USentryTransaction();
};
