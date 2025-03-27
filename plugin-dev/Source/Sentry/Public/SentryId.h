// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryImplWrapper.h"

#include "SentryId.generated.h"

class ISentryId;

/**
 * Unique identifier of the event.
 */
USTRUCT(BlueprintType)
struct SENTRY_API FSentryId
{
	GENERATED_BODY()

	FSentryId();
	FSentryId(TSharedPtr<ISentryId> Id);

	/** Gets string representation of the event ID. */
	FString ToString() const;

	bool IsValid() const { return NativeImpl.IsValid(); }

	TSharedPtr<ISentryId> GetNativeObject() const { return NativeImpl; }

private:
	TSharedPtr<ISentryId> NativeImpl;
};
