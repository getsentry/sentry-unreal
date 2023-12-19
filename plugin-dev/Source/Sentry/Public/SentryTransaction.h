﻿// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTransaction.generated.h"

class ISentryTransaction;
class USentrySpan;

/**
 * Representation of an activity to measure or track.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryTransaction : public UObject
{
	GENERATED_BODY()

public:
	USentryTransaction();

	/** . */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentrySpan* StartChild(const FString& Operation, const FString& Description);

	/** Finishes and sends a transaction to Sentry. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Finish();

	/** Checks whether the transaction finished. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool IsFinished();

	/** Sets transactions name. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetName(const FString& name);

	/** Sets tag associated with the transaction. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTag(const FString& key, const FString& value);

	/** Removes tag associated with the transaction. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& key);

	/** Sets data associated with the transaction. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetData(const FString& key, const TMap<FString, FString>& values);

	/** Removes data associated with the transaction. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveData(const FString& key);

	void InitWithNativeImpl(TSharedPtr<ISentryTransaction> transactionImpl);
	TSharedPtr<ISentryTransaction> GetNativeImpl();

private:
	TSharedPtr<ISentryTransaction> SentryTransactionNativeImpl;
};
