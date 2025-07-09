// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeSendHandler.h"

#include "SentryEvent.h"
#include "SentryHint.h"

USentryEvent* USentryBeforeSendHandler::HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	return Event;
}
