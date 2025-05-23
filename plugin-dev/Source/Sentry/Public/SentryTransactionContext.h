// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "SentryImplWrapper.h"

#include "SentryTransactionContext.generated.h"

class ISentryTransactionContext;

UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryTransactionContext : public UObject, public TSentryImplWrapper<ISentryTransactionContext, USentryTransactionContext>
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the transaction context with the specified name and operation.
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
};
