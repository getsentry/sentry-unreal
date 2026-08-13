// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendFeedbackHandler.h"

#include "CppBeforeSendFeedbackHandler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppBeforeSendFeedbackHandler : public USentryBeforeSendFeedbackHandler
{
	GENERATED_BODY()

public:
	virtual USentryEvent* HandleBeforeSendFeedback_Implementation(USentryEvent* Event, USentryHint* Hint) override;
};
