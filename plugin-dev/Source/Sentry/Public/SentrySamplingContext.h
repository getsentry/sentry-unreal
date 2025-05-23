// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryImplWrapper.h"

#include "SentrySamplingContext.generated.h"

class ISentrySamplingContext;
class USentryTransactionContext;

UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentrySamplingContext : public UObject, public TSentryImplWrapper<ISentrySamplingContext, USentrySamplingContext>
{
	GENERATED_BODY()

public:
	/** Initializes the sampling context. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Gets transaction context used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	USentryTransactionContext* GetTransactionContext() const;

	/** Gets custom data used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetCustomSamplingContext() const;
};
