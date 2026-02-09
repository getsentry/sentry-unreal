// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryUnit.h"
#include "SentryVariant.h"

#include "SentryMetric.generated.h"

class ISentryMetric;

/**
 * Data structure representing a metric entry for Sentry.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryMetric : public UObject, public TSentryImplWrapper<ISentryMetric, USentryMetric>
{
	GENERATED_BODY()

public:
	/** Initializes the metric data. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Sets the metric name. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetName(const FString& Name);

	/** Gets the metric name. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Sets the metric type (counter, gauge, distribution). */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetType(ESentryMetricType Type);

	/** Gets the metric type. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryMetricType GetType() const;

	/** Sets the metric value. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetValue(float Value);

	/** Gets the metric value. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	float GetValue() const;

	/** Sets the metric unit. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetUnit(const FSentryUnit& Unit);

	/** Gets the metric unit. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FSentryUnit GetUnit() const;

	/** Sets an attribute of the metric. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetAttribute(const FString& Key, const FSentryVariant& Value);

	/** Gets an attribute of the metric. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FSentryVariant GetAttribute(const FString& Key) const;

	/** Tries to get an attribute of the metric. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetAttribute(const FString& Key, FSentryVariant& Value) const;

	/** Removes the attribute of the metric with the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveAttribute(const FString& Key);

	/** Adds attributes to the metric. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttributes(const TMap<FString, FSentryVariant>& Attributes);

private:
	static FString MetricTypeToString(ESentryMetricType Type);
	static ESentryMetricType StringToMetricType(const FString& Type);
};
