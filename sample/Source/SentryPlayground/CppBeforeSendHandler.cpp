// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "CppBeforeSendHandler.h"

#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryVariant.h"

USentryEvent* UCppBeforeSendHandler::HandleBeforeSend_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	// Tags
	Event->SetTag(TEXT("before_send.handled"), TEXT("true"));
	Event->RemoveTag(TEXT("tag_to_be_removed"));

	// Extras
	Event->SetExtra(TEXT("handler_added"), FSentryVariant(TEXT("added_value")));
	Event->RemoveExtra(TEXT("extra_to_be_removed"));

	// Contexts
	Event->RemoveContext(TEXT("context_removed_by_handler"));

	return Super::HandleBeforeSend_Implementation(Event, Hint);
}