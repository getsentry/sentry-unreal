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

	/**
	 * Removes attachments whose filename matches the given pattern from event hint.
	 * Only affects the event being processed, scope attachments stay intact.
	 *
	 * @param FilenamePattern Case-insensitive filename pattern supporting `*` and `?` wildcards (e.g. "screenshot*.png").
	 *
	 * @return Number of removed attachments.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	int32 RemoveAttachments(const FString& FilenamePattern);

	/**
	 * Removes all attachments from event hint.
	 * Only affects the event being processed, scope attachments stay intact. Crash dumps are not affected.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearAttachments();
};
