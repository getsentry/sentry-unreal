// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventDesktop.h"
#include "SentryDefines.h"
#include "SentryId.h"
#include "SentryIdDesktop.h"

#include "Infrastructure/SentryConvertersDesktop.h"

#if USE_SENTRY_NATIVE

SentryEventDesktop::SentryEventDesktop()
{
	EventDesktop = sentry_value_new_event();
}

SentryEventDesktop::SentryEventDesktop(sentry_value_t event, bool isCrash)
{
	EventDesktop = event;
	IsCrashEvent = isCrash;
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
	sentry_value_t messageСontainer = sentry_value_new_object();
	sentry_value_set_by_key(messageСontainer, "formatted", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
	sentry_value_set_by_key(EventDesktop, "message", messageСontainer);
}

USentryId* SentryEventDesktop::GetId() const
{
	sentry_value_t id = sentry_value_get_by_key(EventDesktop, "event_id");
	sentry_uuid_t uuid = sentry_uuid_from_string(sentry_value_as_string(id));
	TSharedPtr<ISentryId> idNativeImpl = MakeShareable(new SentryIdDesktop(uuid));
	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);
	return unrealId;
}

FString SentryEventDesktop::GetMessage() const
{
	sentry_value_t messageСontainer = sentry_value_get_by_key(EventDesktop, "message");
	sentry_value_t message = sentry_value_get_by_key(messageСontainer, "formatted");
	return FString(sentry_value_as_string(message));
}

void SentryEventDesktop::SetLevel(ESentryLevel level)
{
	FString levelStr = SentryConvertersDesktop::SentryLevelToString(level).ToLower();
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(EventDesktop, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel SentryEventDesktop::GetLevel() const
{
	sentry_value_t level = sentry_value_get_by_key(EventDesktop, "level");
	return SentryConvertersDesktop::SentryLevelToUnreal(level);
}

bool SentryEventDesktop::IsCrash() const
{
	return IsCrashEvent;
}

bool SentryEventDesktop::IsAnr() const
{
	// ANR error tracking is not available in `sentry-native`
	return false;
}

#endif
