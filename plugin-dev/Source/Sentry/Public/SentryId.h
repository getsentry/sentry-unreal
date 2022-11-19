// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryId.generated.h"

class ISentryId;

/**
 * Unique identifier of the event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryId : public UObject
{
	GENERATED_BODY()

public:
	USentryId();

	/** Gets string representation of the event ID. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString ToString() const;

	void InitWithNativeImpl(TSharedPtr<ISentryId> idImpl);
	TSharedPtr<ISentryId> GetNativeImpl();

private:
	TSharedPtr<ISentryId> SentryIdNativeImpl;
};
