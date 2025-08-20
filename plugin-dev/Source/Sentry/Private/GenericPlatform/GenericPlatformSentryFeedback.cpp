// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryFeedback.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryFeedback::FGenericPlatformSentryFeedback()
{
	Feedback = sentry_value_new_object();
}

FGenericPlatformSentryFeedback::FGenericPlatformSentryFeedback(const FString& message)
{
	Feedback = sentry_value_new_object();
	sentry_value_set_by_key(Feedback, "message", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
}

FGenericPlatformSentryFeedback::~FGenericPlatformSentryFeedback()
{
	// Put custom destructor logic here if needed
}

sentry_value_t FGenericPlatformSentryFeedback::GetNativeObject()
{
	return Feedback;
}

FString FGenericPlatformSentryFeedback::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(Feedback, "message");
	return FString(sentry_value_as_string(message));
}

void FGenericPlatformSentryFeedback::SetName(const FString& name)
{
	sentry_value_set_by_key(Feedback, "name", sentry_value_new_string(TCHAR_TO_UTF8(*name)));
}

FString FGenericPlatformSentryFeedback::GetName() const
{
	sentry_value_t username = sentry_value_get_by_key(Feedback, "name");
	return FString(sentry_value_as_string(username));
}

void FGenericPlatformSentryFeedback::SetContactEmail(const FString& email)
{
	sentry_value_set_by_key(Feedback, "contact_email", sentry_value_new_string(TCHAR_TO_UTF8(*email)));
}

FString FGenericPlatformSentryFeedback::GetContactEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(Feedback, "contact_email");
	return FString(sentry_value_as_string(email));
}

void FGenericPlatformSentryFeedback::SetAssociatedEvent(const FString& eventId)
{
	if (eventId.IsEmpty())
		return;

	sentry_value_set_by_key(Feedback, "associated_event_id", sentry_value_new_string(TCHAR_TO_UTF8(*eventId)));
}

FString FGenericPlatformSentryFeedback::GetAssociatedEvent() const
{
	sentry_value_t comment = sentry_value_get_by_key(Feedback, "associated_event_id");
	return FString(sentry_value_as_string(comment));
}

#endif
