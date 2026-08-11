// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "CppBeforeSendFeedbackHandler.h"

#include "SentryFeedback.h"
#include "SentryHint.h"

USentryFeedback* UCppBeforeSendFeedbackHandler::HandleBeforeSendFeedback_Implementation(USentryFeedback* Feedback, USentryHint* Hint)
{
	Feedback->SetContactEmail(TEXT("redacted@sentry.local"));

	return Super::HandleBeforeSendFeedback_Implementation(Feedback, Hint);
}
