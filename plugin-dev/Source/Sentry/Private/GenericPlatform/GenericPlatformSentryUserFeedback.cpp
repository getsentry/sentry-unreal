// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryUserFeedback.h"

#include "GenericPlatformSentryId.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryUserFeedback::FGenericPlatformSentryUserFeedback()
{
	UserFeedback = sentry_value_new_object();
}

FGenericPlatformSentryUserFeedback::FGenericPlatformSentryUserFeedback(TSharedPtr<ISentryId> eventId)
{
	UserFeedback = sentry_value_new_object();
	sentry_value_set_by_key(UserFeedback, "event_id", sentry_value_new_string(TCHAR_TO_ANSI(*eventId->ToString())));
}

FGenericPlatformSentryUserFeedback::~FGenericPlatformSentryUserFeedback()
{
	// Put custom destructor logic here if needed
}

sentry_value_t FGenericPlatformSentryUserFeedback::GetNativeObject()
{
	return UserFeedback;
}

void FGenericPlatformSentryUserFeedback::SetName(const FString& name)
{
	sentry_value_set_by_key(UserFeedback, "name", sentry_value_new_string(TCHAR_TO_UTF8(*name)));
}

FString FGenericPlatformSentryUserFeedback::GetName() const
{
	sentry_value_t username = sentry_value_get_by_key(UserFeedback, "name");
	return FString(sentry_value_as_string(username));
}

void FGenericPlatformSentryUserFeedback::SetEmail(const FString& email)
{
	sentry_value_set_by_key(UserFeedback, "email", sentry_value_new_string(TCHAR_TO_ANSI(*email)));
}

FString FGenericPlatformSentryUserFeedback::GetEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(UserFeedback, "email");
	return FString(sentry_value_as_string(email));
}

void FGenericPlatformSentryUserFeedback::SetComment(const FString& comment)
{
	sentry_value_set_by_key(UserFeedback, "comments", sentry_value_new_string(TCHAR_TO_UTF8(*comment)));
}

FString FGenericPlatformSentryUserFeedback::GetComment() const
{
	sentry_value_t comment = sentry_value_get_by_key(UserFeedback, "comments");
	return FString(sentry_value_as_string(comment));
}

#endif
