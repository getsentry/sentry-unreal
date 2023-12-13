// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SentrySpan.generated.h"

class ISentrySpan;

/**
 * Unit of work within a transaction.
 */
UCLASS(BlueprintType)
class SENTRY_API USentrySpan : public UObject
{
	GENERATED_BODY()

public:
	USentrySpan();

	/** Finishes and sends a span to Sentry. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Finish();

	void InitWithNativeImpl(TSharedPtr<ISentrySpan> spanImpl);
	TSharedPtr<ISentrySpan> GetNativeImpl();

private:
	TSharedPtr<ISentrySpan> SentrySpanNativeImpl;
};
