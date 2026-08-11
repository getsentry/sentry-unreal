// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryBeforeSendFeedbackHandler.h"

#include "SentryFeedback.h"
#include "SentryHint.h"

USentryFeedback* USentryBeforeSendFeedbackHandler::HandleBeforeSendFeedback_Implementation(USentryFeedback* Feedback, USentryHint* Hint)
{
	return Feedback;
}
