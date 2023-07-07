// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "SentryHint.generated.h"

class ISentryHint;
class USentryAttachment;

/**
 * Hint associated with the event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryHint : public UObject
{
	GENERATED_BODY()

public:
	USentryHint();

	/** Adds attachment to event hint. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttachment(USentryAttachment* Attachment);

	void InitWithNativeImpl(TSharedPtr<ISentryHint> hintImpl);
	TSharedPtr<ISentryHint> GetNativeImpl();

private:
	TSharedPtr<ISentryHint> SentryHintNativeImpl;
};
