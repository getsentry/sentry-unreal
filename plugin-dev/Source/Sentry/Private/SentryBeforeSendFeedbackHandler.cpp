// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBeforeSendFeedbackHandler.h"

#include "SentryEvent.h"
#include "SentryHint.h"

USentryEvent* USentryBeforeSendFeedbackHandler::HandleBeforeSendFeedback_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	return Event;
}
