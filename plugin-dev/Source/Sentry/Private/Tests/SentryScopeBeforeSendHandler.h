// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryBeforeSendHandler.h"
#include "SentryEvent.h"
#include "SentryHint.h"

#include "SentryScopeBeforeSendHandler.generated.h"

UCLASS()
class UScopeTestBeforeSendHandler : public USentryBeforeSendHandler
{
	GENERATED_BODY()
public:
	virtual USentryEvent* HandleBeforeSend_Implementation(USentryEvent* SentryEvent, USentryHint* Hint) override
	{
		OnScopeTestBeforeSendHandler.ExecuteIfBound(SentryEvent);
		return Super::HandleBeforeSend_Implementation(SentryEvent, Hint);
	}

	static TDelegate<void(USentryEvent*)> OnScopeTestBeforeSendHandler;
};
