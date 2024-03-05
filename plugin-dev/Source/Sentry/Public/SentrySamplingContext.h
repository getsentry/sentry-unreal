// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentrySamplingContext.generated.h"

class ISentrySamplingContext;
class USentryTransactionContext;

UCLASS(BlueprintType)
class SENTRY_API USentrySamplingContext : public UObject
{
	GENERATED_BODY()

public:
	USentrySamplingContext();

	/** Gets transaction context used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	USentryTransactionContext* GetTransactionContext() const;

	/** Gets custom data used for sampling. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetCustomSamplingContext() const;

	void InitWithNativeImpl(TSharedPtr<ISentrySamplingContext> samplingContextImpl);
	TSharedPtr<ISentrySamplingContext> GetNativeImpl();

private:
	TSharedPtr<ISentrySamplingContext> SentrySamplingContextNativeImpl;
};
