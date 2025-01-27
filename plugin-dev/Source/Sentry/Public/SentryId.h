// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryId.generated.h"

class ISentryId;

/**
 * Unique identifier of the event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryId : public UObject, public TSentryImplWrapper<ISentryId, USentryId>
{
	GENERATED_BODY()

public:
	/** Initializes the identifier. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Gets string representation of the event ID. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString ToString() const;
};
