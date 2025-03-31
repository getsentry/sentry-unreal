// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryBreadcrumb.generated.h"

class ISentryBreadcrumb;

/**
 * Information to create a trail of events that happened prior to an issue.
 */
USTRUCT(BlueprintType)
struct SENTRY_API FSentryBreadcrumb
{
	GENERATED_BODY()

	FSentryBreadcrumb();

	void SetMessage(const FString& Message);
	FString GetMessage() const;

	void SetType(const FString& Type);
	FString GetType() const;

	void SetCategory(const FString& Category);
	FString GetCategory() const;

	void SetData(const TMap<FString, FString>& Data);
	TMap<FString, FString> GetData() const;

	void SetLevel(ESentryLevel Level);
	ESentryLevel GetLevel() const;

	/** Retrieves the underlying native implementation. */
	TSharedPtr<ISentryBreadcrumb> GetNativeObject() const { return NativeImpl; }

private:
	TSharedPtr<ISentryBreadcrumb> NativeImpl;
};

/**
 * Utility blueprint functions for Sentry breadcrumb.
 */
UCLASS()
class SENTRY_API USentryBreadcrumbLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets message of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetMessage(UPARAM(ref) FSentryBreadcrumb& Breadcrumb, const FString& Message);

	/** Gets message of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FString GetMessage(const FSentryBreadcrumb& Breadcrumb);

	/** Sets type of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetType(UPARAM(ref) FSentryBreadcrumb& Breadcrumb, const FString& Type);

	/** Gets type of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FString GetType(const FSentryBreadcrumb& Breadcrumb);

	/** Sets category of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetCategory(UPARAM(ref) FSentryBreadcrumb& Breadcrumb, const FString& Category);

	/** Gets category of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static FString GetCategory(const FSentryBreadcrumb& Breadcrumb);

	/** Sets data associated with the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetData(UPARAM(ref) FSentryBreadcrumb& Breadcrumb, const TMap<FString, FString>& Data);

	/** Gets data associated with the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static TMap<FString, FString> GetData(const FSentryBreadcrumb& Breadcrumb);

	/** Sets the level of the breadcrumb. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static void SetLevel(UPARAM(ref) FSentryBreadcrumb& Breadcrumb, ESentryLevel Level);

	/** Gets the level of the breadcrumb. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static ESentryLevel GetLevel(const FSentryBreadcrumb& Breadcrumb);
};
