// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventDesktop.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

SentryEventDesktop::SentryEventDesktop()
{
	EventDesktop = sentry_value_new_event();
}

SentryEventDesktop::SentryEventDesktop(sentry_value_t event)
{
	EventDesktop = event;
}

SentryEventDesktop::~SentryEventDesktop()
{
	// Put custom destructor logic here if needed
}

sentry_value_t SentryEventDesktop::GetNativeObject()
{
	return EventDesktop;
}

void SentryEventDesktop::SetMessage(const FString& message)
{
	sentry_value_set_by_key(EventDesktop, "message", sentry_value_new_string(TCHAR_TO_ANSI(*message)));
}

FString SentryEventDesktop::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(EventDesktop, "message");
	return FString(sentry_value_as_string(message));
}

void SentryEventDesktop::SetLevel(ESentryLevel level)
{
	FString levelStr = SentryConvertorsDesktop::SentryLevelToString(level);
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(EventDesktop, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel SentryEventDesktop::GetLevel() const
{
	sentry_value_t level = sentry_value_get_by_key(EventDesktop, "level");
	return SentryConvertorsDesktop::SentryLevelToUnreal(level);
}
