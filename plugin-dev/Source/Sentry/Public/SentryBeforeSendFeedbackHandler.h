// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendFeedbackHandler.generated.h"

class USentryFeedback;
class USentryHint;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeSendFeedbackHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryFeedback* HandleBeforeSendFeedback(USentryFeedback* Feedback, USentryHint* Hint);
	virtual USentryFeedback* HandleBeforeSendFeedback_Implementation(USentryFeedback* Feedback, USentryHint* Hint);
};
