// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentrySpan.generated.h"

/**
 * Unit of work within a transaction.
 */
UCLASS(BlueprintType)
class SENTRY_API USentrySpan : public UObject
{
	GENERATED_BODY()

public:
	USentrySpan();
};
