// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryHint.generated.h"

class ISentryHint;
class USentryAttachment;

/**
 * Hint associated with the event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryHint : public UObject, public TSentryImplWrapper<ISentryHint, USentryHint>
{
	GENERATED_BODY()

public:
	/** Adds attachment to event hint. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttachment(USentryAttachment* Attachment);
};
