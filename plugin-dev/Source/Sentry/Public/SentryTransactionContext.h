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

	/** Sets transaction name. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetName(const FString& Name);

	/** Gets transaction name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Sets transaction origin. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetOrigin(const FString& Origin);

	/** Gets transaction origin. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOrigin() const;

	/** Sets operation name. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetOperation(const FString& Operation);

	/** Gets operation name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetOperation() const;

	void InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl);
	TSharedPtr<ISentryTransactionContext> GetNativeImpl();

private:
	TSharedPtr<ISentryTransactionContext> SentryTransactionContextNativeImpl;
};
