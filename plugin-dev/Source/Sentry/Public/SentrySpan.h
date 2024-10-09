﻿// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentrySpan.generated.h"

class ISentrySpan;

/**
 * Unit of work within a transaction.
 */
UCLASS(BlueprintType)
class SENTRY_API USentrySpan : public UObject
{
	GENERATED_BODY()

public:
	USentrySpan();

	/** Finishes and sends a span to Sentry. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Finish();

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
	void SetData(const FString& key, const TMap<FString, FString>& values);

	/** Removes data associated with the span. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveData(const FString& key);

	/** Gets trace information that could be sent as a `sentry-trace` header */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void GetTrace(FString& name, FString& value);

	void InitWithNativeImpl(TSharedPtr<ISentrySpan> spanImpl);
	TSharedPtr<ISentrySpan> GetNativeImpl();

private:
	TSharedPtr<ISentrySpan> SentrySpanNativeImpl;
};
