// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryBeforeSendHandler.generated.h"

class USentryEvent;
class USentryHint;

UCLASS(Blueprintable)
class USentryBeforeSendHandler : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void HandleBeforeSend(USentryEvent* Event, USentryHint* Hint);
	virtual void HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint);
};