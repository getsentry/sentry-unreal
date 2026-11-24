// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryVariant.h"

#include "SentryTransactionOptions.generated.h"

/**
 * Additional data used to create transactions.
 */
USTRUCT(BlueprintType)
struct SENTRY_API FSentryTransactionOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Sentry")
	TMap<FString, FSentryVariant> CustomSamplingContext;

	UPROPERTY(BlueprintReadWrite, Category = "Sentry")
	bool BindToScope = false;
};
