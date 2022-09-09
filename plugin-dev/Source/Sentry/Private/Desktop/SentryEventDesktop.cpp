// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventDesktop.h"
#include "SentryDefines.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

SentryEventDesktop::SentryEventDesktop()
{
	EventDesktop = sentry_value_new_event();
	sentry_value_freeze(EventDesktop);
}

SentryEventDesktop::SentryEventDesktop(sentry_value_t event)
{
	EventDesktop = event;
}

SentryEventDesktop::~SentryEventDesktop()
{
	// Put custom destructor logic here if needed
	UE_LOG(LogSentrySdk, Log, TEXT("EventDesktop destructor"));
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
	FString levelStr = SentryConvertorsDesktop::SentryLevelToString(level).ToLower();
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(EventDesktop, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel SentryEventDesktop::GetLevel() const
{
	if(sentry_value_refcount(EventDesktop) == 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("EventDesktop ref count equals zero"));
	}
	else
	{
		UE_LOG(LogSentrySdk, Log, TEXT("EventDesktop ref count is not zero"));
	}

	sentry_value_t level = sentry_value_get_by_key(EventDesktop, "level");
	return SentryConvertorsDesktop::SentryLevelToUnreal(level);
}
