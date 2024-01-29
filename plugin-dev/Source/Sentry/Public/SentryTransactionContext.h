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

	/**
	 * Initializes transaction context with specified name and operation.
	 *
	 * @param Name Transaction name.
	 * @param Operation Operation name.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize(const FString& Name, const FString& Operation);

	/** Gets transaction name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Gets operation name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOperation() const;

	void InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl);
	TSharedPtr<ISentryTransactionContext> GetNativeImpl();

private:
	TSharedPtr<ISentryTransactionContext> SentryTransactionContextNativeImpl;
};
