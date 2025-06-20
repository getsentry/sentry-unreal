// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryImplWrapper.h"

#include "SentrySamplingContext.generated.h"

class ISentrySamplingContext;
class USentryTransactionContext;

/**
 * Context used by trace sampler to determine if transaction is going to be sampled.
 *
 * NOTE: USentrySamplingContext should not be constructed with NewObject<...>() etc., and should instead
 *       only be created internally by the SDK using the platform-specific implementations.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentrySamplingContext : public UObject, public TSentryImplWrapper<ISentrySamplingContext, USentrySamplingContext>
{
	GENERATED_BODY()

public:
	/** Gets transaction context used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	USentryTransactionContext* GetTransactionContext() const;

	/** Gets custom data used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetCustomSamplingContext() const;
};
