// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryHint.generated.h"

class ISentryHint;
class USentryAttachment;

/**
 * Hint associated with the event.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryHint : public UObject, public TSentryImplWrapper<ISentryHint, USentryHint>
{
	GENERATED_BODY()

public:
	/** Initializes the hint. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Adds attachment to event hint. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttachment(USentryAttachment* Attachment);
};
