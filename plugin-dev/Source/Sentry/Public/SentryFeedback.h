// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryFeedback.generated.h"

class ISentryFeedback;

/**
 * Additional information about what happened to be sent to Sentry.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryFeedback : public UObject, public TSentryImplWrapper<ISentryFeedback, USentryFeedback>
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the user feedback with the provided message.
	 *
	 * @param Message The user feedback message to record.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize(const FString& Message);

	/** Gets the feedback message. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetMessage() const;

	/** Sets the name of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetName(const FString& Name);

	/** Gets the name of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetName() const;

	/** Sets the email of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContactEmail(const FString& Email);

	/** Gets the email of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetContactEmail() const;

	/** Sets associated event identifier. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetAssociatedEvent(const FString& EventId);

	/** Gets associated event identifier. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetAssociatedEvent() const;
};
