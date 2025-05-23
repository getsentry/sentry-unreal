// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendHandler.h"

#include "CppBeforeSendHandler.generated.h"

UCLASS()
class SENTRYPLAYGROUND_API UCppBeforeSendHandler : public USentryBeforeSendHandler
{
	GENERATED_BODY()

public:
	virtual USentryEvent* HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint) override;
};
