// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentryLogData.generated.h"

class ISentryLog;

/**
 * Data structure representing a structured log entry for Sentry.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryLogData : public UObject, public TSentryImplWrapper<ISentryLog, USentryLogData>
{
	GENERATED_BODY()

public:
	/** Initializes the log data. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Sets the log message. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetMessage(const FString& Message);

	/** Gets the log message. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetMessage() const;

	/** Sets the log level. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the log level. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

private:
	UPROPERTY()
	FString Message;

	UPROPERTY()
	ESentryLevel Level;
};