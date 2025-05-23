// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryEvent.h"
#include "GenericPlatformSentryId.h"

#include "SentryDefines.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryEvent::FGenericPlatformSentryEvent()
{
	Event = sentry_value_new_event();
}

FGenericPlatformSentryEvent::FGenericPlatformSentryEvent(sentry_value_t event, bool isCrash)
{
	Event = event;
	IsCrashEvent = isCrash;
}

FGenericPlatformSentryEvent::~FGenericPlatformSentryEvent()
{
	// Put custom destructor logic here if needed
}

sentry_value_t FGenericPlatformSentryEvent::GetNativeObject()
{
	return Event;
}

TSharedPtr<ISentryId> FGenericPlatformSentryEvent::GetId() const
{
	sentry_value_t eventId = sentry_value_get_by_key(Event, "event_id");
	sentry_uuid_t uuid = sentry_uuid_from_string(sentry_value_as_string(eventId));
	return MakeShareable(new FGenericPlatformSentryId(uuid));
}

void FGenericPlatformSentryEvent::SetMessage(const FString& message)
{
	sentry_value_t messageСontainer = sentry_value_new_object();
	sentry_value_set_by_key(messageСontainer, "formatted", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
	sentry_value_set_by_key(Event, "message", messageСontainer);
}

FString FGenericPlatformSentryEvent::GetMessage() const
{
	sentry_value_t messageСontainer = sentry_value_get_by_key(Event, "message");
	sentry_value_t message = sentry_value_get_by_key(messageСontainer, "formatted");
	return FString(sentry_value_as_string(message));
}

void FGenericPlatformSentryEvent::SetLevel(ESentryLevel level)
{
	FString levelStr = FGenericPlatformSentryConverters::SentryLevelToString(level).ToLower();
	if (!levelStr.IsEmpty())
		sentry_value_set_by_key(Event, "level", sentry_value_new_string(TCHAR_TO_ANSI(*levelStr)));
}

ESentryLevel FGenericPlatformSentryEvent::GetLevel() const
{
	sentry_value_t level = sentry_value_get_by_key(Event, "level");
	return FGenericPlatformSentryConverters::SentryLevelToUnreal(level);
}

void FGenericPlatformSentryEvent::SetFingerprint(const TArray<FString>& fingerprint)
{
	sentry_value_set_by_key(Event, "fingerprint", FGenericPlatformSentryConverters::StringArrayToNative(fingerprint));
}

TArray<FString> FGenericPlatformSentryEvent::GetFingerprint()
{
	sentry_value_t fingerprint = sentry_value_get_by_key(Event, "fingerprint");
	return FGenericPlatformSentryConverters::StringArrayToUnreal(fingerprint);
}

bool FGenericPlatformSentryEvent::IsCrash() const
{
	return IsCrashEvent;
}

bool FGenericPlatformSentryEvent::IsAnr() const
{
	// ANR error tracking is not available in `sentry-native`
	return false;
}

#endif
