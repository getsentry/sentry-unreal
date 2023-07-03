// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryEvent.generated.h"

class ISentryEvent;

/**
 * Data being sent to Sentry.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryEvent : public UObject
{
	GENERATED_BODY()

public:
	USentryEvent();

	/** Sets message of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetMessage(const FString& Message);

	/** Gets message of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetMessage() const;

	/** Sets the level of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the level of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

	/** Gets flag indicating whether the event is a crash. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool IsCrash() const;

	void InitWithNativeImpl(TSharedPtr<ISentryEvent> eventImpl);
	TSharedPtr<ISentryEvent> GetNativeImpl();

private:
	TSharedPtr<ISentryEvent> EventNativeImpl;
};
