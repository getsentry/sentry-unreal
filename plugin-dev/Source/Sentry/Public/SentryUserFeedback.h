// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryUserFeedback.generated.h"

class USentryId;
class ISentryUserFeedback;

/**
 * Additional information about what happened to an event.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryUserFeedback : public UObject, public TSentryImplWrapper<ISentryUserFeedback, USentryUserFeedback>
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the user feedback with the event identifier to which it is associated.
	 *
	 * @param EventId The associated event identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize(const FString& EventId);

	/** Sets the name of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetName(const FString& Name);

	/** Gets the name of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Sets the email of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetEmail(const FString& Email);

	/** Gets the email of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetEmail() const;

	/** Sets comments of the user about what happened. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetComment(const FString& Comments);

	/** Gets comments of the user about what happened. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetComment() const;
};
