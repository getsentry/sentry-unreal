// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentryBreadcrumb.generated.h"

class ISentryBreadcrumb;

/**
 * Information to create a trail of events that happened prior to an issue.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryBreadcrumb : public UObject, public TSentryImplWrapper<ISentryBreadcrumb, USentryBreadcrumb>
{
	GENERATED_BODY()

public:
	/** Initializes the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Sets message of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetMessage(const FString& Message);

	/** Gets message of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetMessage() const;

	/** Sets type of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetType(const FString& Type);

	/** Gets type of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetType() const;

	/** Sets category of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetCategory(const FString& Category);

	/** Gets category of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetCategory() const;

	/** Sets data associated with the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetData(const TMap<FString, FSentryVariant>& Data);

	/** Gets data associated with the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FSentryVariant> GetData() const;

	/** Sets the level of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the level of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;
};
