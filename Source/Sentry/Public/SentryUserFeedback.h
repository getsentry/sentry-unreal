// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryUserFeedback.generated.h"

class USentryId;
class ISentryUserFeedback;

/**
 * Additional information about what happened to an event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryUserFeedback : public UObject
{
	GENERATED_BODY()

public:

	/** Initializes user feedback with Id of the event to which it is associated. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize(USentryId* EventId);

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

	void InitWithNativeImpl(TSharedPtr<ISentryUserFeedback> userFeedbackImpl);
	TSharedPtr<ISentryUserFeedback> GetNativeImpl();

private:
	TSharedPtr<ISentryUserFeedback> UserFeedbackNativeImpl;
};