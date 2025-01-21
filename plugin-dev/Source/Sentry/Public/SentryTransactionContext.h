// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "SentryImplWrapper.h"

#include "SentryTransactionContext.generated.h"

class ISentryTransactionContext;

UCLASS(BlueprintType)
class SENTRY_API USentryTransactionContext : public UObject, public TSentryImplWrapper<ISentryTransactionContext, USentryTransactionContext>
{
	GENERATED_BODY()

public:
	/** Gets transaction name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Gets operation name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOperation() const;
};
