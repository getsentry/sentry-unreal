// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppBeforeSendHandler.h"

#include "SentryEvent.h"
#include "SentryHint.h"

USentryEvent* UCppBeforeSendHandler::HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	UE_LOG(LogTemp, Log, TEXT("Hello from CPP beforeSend handler"));

	return Super::HandleBeforeSend_Implementation(Event, Hint);
}