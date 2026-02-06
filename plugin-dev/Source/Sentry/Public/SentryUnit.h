// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryUnit.generated.h"

/**
 * Predefined measurement units for Sentry metrics.
 */
UENUM(BlueprintType)
enum class ESentryUnit : uint8
{
	None,

	// Duration
	Nanosecond,
	Microsecond,
	Millisecond,
	Second,
	Minute,
	Hour,
	Day,
	Week,

	// Information
	Bit,
	Byte,
	Kilobyte,
	Kibibyte,
	Megabyte,
	Mebibyte,
	Gigabyte,
	Gibibyte,
	Terabyte,
	Tebibyte,
	Petabyte,
	Pebibyte,
	Exabyte,
	Exbibyte,

	// Fraction
	Ratio,
	Percent
};

/**
 * A measurement unit that can be attached to a metric value.
 * Use predefined units from the enum or provide a custom string.
 */
USTRUCT(BlueprintType, meta = (HasNativeMake = "/Script/Sentry.SentryUnitHelper.MakeSentryUnit"))
struct SENTRY_API FSentryUnit
{
	GENERATED_BODY()

	FSentryUnit();
	FSentryUnit(ESentryUnit InUnit);
	FSentryUnit(const FString& InCustomUnit);

	/** Converts the unit to its string representation. */
	FString ToString() const;

private:
	ESentryUnit Unit;
	FString CustomUnit;
};

/**
 * Blueprint factory functions for FSentryUnit.
 */
UCLASS()
class SENTRY_API USentryUnitHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates a measurement unit from a predefined type. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FSentryUnit MakeSentryUnit(ESentryUnit Unit);

	/** Creates a measurement unit from a custom string. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FSentryUnit MakeSentryCustomUnit(const FString& Unit);

	/** Converts the unit to its string representation. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FString ToString(const FSentryUnit& Unit);
};
