// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryMeasurementUnit.generated.h"

/**
 * Predefined measurement units for Sentry metrics.
 */
UENUM(BlueprintType)
enum class ESentryMeasurementUnit : uint8
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
	Percent,
	// Custom string
	Custom
};

/**
 * A measurement unit that can be attached to a metric value.
 * Use predefined units from the enum or provide a custom string.
 */
USTRUCT(BlueprintType)
struct SENTRY_API FSentryMeasurementUnit
{
	GENERATED_BODY()

	/** The predefined measurement unit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sentry")
	ESentryMeasurementUnit Unit = ESentryMeasurementUnit::None;

	/** Custom unit string, used when Unit is set to Custom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sentry", meta = (EditCondition = "Unit==ESentryMeasurementUnit::Custom"))
	FString CustomUnit;

	FSentryMeasurementUnit() = default;
	FSentryMeasurementUnit(ESentryMeasurementUnit InUnit);
	FSentryMeasurementUnit(const FString& InCustomUnit);

	/** Converts the unit to its string representation. */
	FString ToString() const;
};
