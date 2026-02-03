// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentryLog.generated.h"

class ISentryLog;

/**
 * Data structure representing a structured log entry for Sentry.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryLog : public UObject, public TSentryImplWrapper<ISentryLog, USentryLog>
{
	GENERATED_BODY()

public:
	/** Initializes the log data. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Sets the log body. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetBody(const FString& Body);

	/** Gets the log body. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetBody() const;

	/** Sets the log level. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the log level. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

	/** Sets an attribute of the log. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetAttribute(const FString& Key, const FSentryVariant& Value);

	/** Gets an attribute of the log. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FSentryVariant GetAttribute(const FString& Key) const;

	/** Tries to get an attribute of the log. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetAttribute(const FString& Key, FSentryVariant& Value) const;

	/** Removes the attribute of the log with the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveAttribute(const FString& Key);

	/** Adds attributes to the log. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttributes(const TMap<FString, FSentryVariant>& Attributes);

private:
	UPROPERTY()
	FString Body;

	UPROPERTY()
	ESentryLevel Level;
};