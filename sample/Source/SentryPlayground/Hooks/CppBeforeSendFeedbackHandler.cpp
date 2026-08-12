// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "CppBeforeSendFeedbackHandler.h"

#include "SentryEvent.h"
#include "SentryHint.h"
#include "SentryVariant.h"

USentryEvent* UCppBeforeSendFeedbackHandler::HandleBeforeSendFeedback_Implementation(USentryEvent* Event, USentryHint* Hint)
{
	TMap<FString, FSentryVariant> Feedback = Event->GetContext(TEXT("feedback"));
	Feedback.Add(TEXT("contact_email"), FSentryVariant(TEXT("redacted@sentry.local")));
	Event->SetContext(TEXT("feedback"), Feedback);

	return Super::HandleBeforeSendFeedback_Implementation(Event, Hint);
}
