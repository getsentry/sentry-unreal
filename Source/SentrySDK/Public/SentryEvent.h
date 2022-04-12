// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryEvent.generated.h"

class ISentryEvent;

UCLASS(BlueprintType)
class SENTRYSDK_API USentryEvent : public UObject
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

	void InitWithNativeImpl(TSharedPtr<ISentryEvent> eventImpl);
	TSharedPtr<ISentryEvent> GetNativeImpl();

private:
	TSharedPtr<ISentryEvent> EventNativeImpl;
};
