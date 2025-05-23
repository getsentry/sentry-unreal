// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendHandler.generated.h"

class USentryEvent;
class USentryHint;

UCLASS(Blueprintable)
class SENTRY_API USentryBeforeSendHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USentryEvent* HandleBeforeSend(USentryEvent* Event, USentryHint* Hint);
	virtual USentryEvent* HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint);
};