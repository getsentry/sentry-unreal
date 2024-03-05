// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryBreadcrumb.generated.h"

class ISentryBreadcrumb;

/**
 * Information to create a trail of events that happened prior to an issue.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryBreadcrumb : public UObject
{
	GENERATED_BODY()

public:
	USentryBreadcrumb();

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
	void SetData(const TMap<FString, FString>& Data);

	/** Gets data associated with the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetData() const;

	/** Sets the level of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the level of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

	void InitWithNativeImpl(TSharedPtr<ISentryBreadcrumb> breadcrumbImpl);
	TSharedPtr<ISentryBreadcrumb> GetNativeImpl();

private:
	TSharedPtr<ISentryBreadcrumb> BreadcrumbNativeImpl;
};