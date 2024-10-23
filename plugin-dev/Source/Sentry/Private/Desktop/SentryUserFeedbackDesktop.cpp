// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackDesktop.h"

#include "SentryIdDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#if USE_SENTRY_NATIVE

SentryUserFeedbackDesktop::SentryUserFeedbackDesktop()
{
	UserFeedbackDesktop = sentry_value_new_object();
}

SentryUserFeedbackDesktop::SentryUserFeedbackDesktop(TSharedPtr<ISentryId> eventId)
{
	UserFeedbackDesktop = sentry_value_new_object();
	sentry_value_set_by_key(UserFeedbackDesktop, "event_id", sentry_value_new_string(TCHAR_TO_ANSI(*eventId->ToString())));
}

SentryUserFeedbackDesktop::~SentryUserFeedbackDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_value_t SentryUserFeedbackDesktop::GetNativeObject()
{
	return UserFeedbackDesktop;
}

void SentryUserFeedbackDesktop::SetName(const FString& name)
{
	sentry_value_set_by_key(UserFeedbackDesktop, "name", sentry_value_new_string(TCHAR_TO_UTF8(*name)));
}

FString SentryUserFeedbackDesktop::GetName() const
{
	sentry_value_t username = sentry_value_get_by_key(UserFeedbackDesktop, "name");
	return FString(sentry_value_as_string(username));
}

void SentryUserFeedbackDesktop::SetEmail(const FString& email)
{
	sentry_value_set_by_key(UserFeedbackDesktop, "email", sentry_value_new_string(TCHAR_TO_ANSI(*email)));
}

FString SentryUserFeedbackDesktop::GetEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(UserFeedbackDesktop, "email");
	return FString(sentry_value_as_string(email));
}

void SentryUserFeedbackDesktop::SetComment(const FString& comment)
{
	sentry_value_set_by_key(UserFeedbackDesktop, "comments", sentry_value_new_string(TCHAR_TO_UTF8(*comment)));
}

FString SentryUserFeedbackDesktop::GetComment() const
{
	sentry_value_t comment = sentry_value_get_by_key(UserFeedbackDesktop, "comments");
	return FString(sentry_value_as_string(comment));
}

#endif