// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "CppBeforeSendFeedbackHandler.h"

#include "SentryEvent.h"
#include "SentryFeedback.h"
#include "SentryHint.h"

USentryEvent* UCppBeforeSendFeedbackHandler::HandleBeforeSendFeedback_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	if (USentryFeedback* Feedback = Event->GetFeedback())
	{
		Feedback->SetContactEmail(TEXT("redacted@sentry.local"));
	}

	return Super::HandleBeforeSendFeedback_Implementation(Event, Hint);
}
