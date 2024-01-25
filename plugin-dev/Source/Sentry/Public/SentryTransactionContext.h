// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTransactionContext.generated.h"

class ISentryTransactionContext;

UCLASS(BlueprintType)
class SENTRY_API USentryTransactionContext : public UObject
{
	GENERATED_BODY()

public:
	USentryTransactionContext();

	/** Gets transaction name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Gets transaction origin. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOrigin() const;

	/** Gets operation name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOperation() const;

	void InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl);
	TSharedPtr<ISentryTransactionContext> GetNativeImpl();

private:
	TSharedPtr<ISentryTransactionContext> SentryTransactionContextNativeImpl;
};
