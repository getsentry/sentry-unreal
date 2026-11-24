// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendFeedbackHandler.generated.h"

class USentryEvent;
class USentryHint;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeSendFeedbackHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryEvent* HandleBeforeSendFeedback(USentryEvent* Event, USentryHint* Hint);
	virtual USentryEvent* HandleBeforeSendFeedback_Implementation(USentryEvent* Event, USentryHint* Hint);
};
