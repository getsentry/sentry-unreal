// Copyright (c) 2024 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentryTransactionContext.generated.h"

class ISentryTransactionContext;

UCLASS(BlueprintType)
class SENTRY_API USentryTransactionContext : public UObject
{
	GENERATED_BODY()

public:
	USentryTransactionContext();

	void InitWithNativeImpl(TSharedPtr<ISentryTransactionContext> transactionContextImpl);
	TSharedPtr<ISentryTransactionContext> GetNativeImpl();

private:
	TSharedPtr<ISentryTransactionContext> SentryTransactionNativeImpl;
};
