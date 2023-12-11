// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTransaction.generated.h"

class ISentryTransaction;

/**
 * Representation of an activity to measure or track.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryTransaction : public UObject
{
	GENERATED_BODY()

public:
	USentryTransaction();

	void InitWithNativeImpl(TSharedPtr<ISentryTransaction> transactionImpl);
	TSharedPtr<ISentryTransaction> GetNativeImpl();

private:
	TSharedPtr<ISentryTransaction> SentryTransactionNativeImpl;
};
