// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentrySpan.generated.h"

class ISentrySpan;

/**
 * Unit of work within a transaction.
 *
 * NOTE: USentrySpan should not be constructed with NewObject<...>() etc., and should instead
 *       only be created by calling methods like StartChild(...) on this object or USentryTransaction.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentrySpan : public UObject, public TSentryImplWrapper<ISentrySpan, USentrySpan>
{
	GENERATED_BODY()

public:
	/** Starts a new child span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentrySpan* StartChild(const FString& Operation, const FString& Description);

	/** Starts a new child span with timestamp. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentrySpan* StartChildWithTimestamp(const FString& Operation, const FString& Description, int64 Timestamp);

	/** Finishes and sends a span to Sentry. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Finish();

	/** Finishes with timestamp and sends a span to Sentry. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void FinishWithTimestamp(int64 Timestamp);

	/** Checks whether the span finished. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool IsFinished() const;

	/** Sets tag associated with the span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTag(const FString& key, const FString& value);

	/** Removes tag associated with the span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& key);

	/** Sets data associated with the span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetData(const FString& key, const TMap<FString, FSentryVariant>& values);

	/** Removes data associated with the span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveData(const FString& key);

	/** Gets trace information that could be sent as a `sentry-trace` header */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void GetTrace(FString& name, FString& value);
};
