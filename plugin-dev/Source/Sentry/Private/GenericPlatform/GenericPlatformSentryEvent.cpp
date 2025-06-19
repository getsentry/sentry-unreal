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

void FGenericPlatformSentryEvent::SetTag(const FString& key, const FString& value)
{
	sentry_value_t eventTags = sentry_value_get_by_key(Event, "tags");
	if (sentry_value_is_null(eventTags))
	{
		sentry_value_set_by_key(Event, "tags", FGenericPlatformSentryConverters::StringMapToNative({ { key, value } }));
	}
	else
	{
		sentry_value_set_by_key(eventTags, TCHAR_TO_ANSI(*key), sentry_value_new_string(TCHAR_TO_ANSI(*value)));
	}
}

FString FGenericPlatformSentryEvent::GetTag(const FString& key) const
{
	sentry_value_t eventTags = sentry_value_get_by_key(Event, "tags");
	if (sentry_value_is_null(eventTags))
	{
		return FString();
	}

	sentry_value_t tag = sentry_value_get_by_key(eventTags, TCHAR_TO_ANSI(*key));
	return FString(sentry_value_as_string(tag));
}

bool FGenericPlatformSentryEvent::TryGetTag(const FString& key, FString& value) const
{
	sentry_value_t eventTags = sentry_value_get_by_key(Event, "tags");
	if (sentry_value_is_null(eventTags))
	{
		return false;
	}

	sentry_value_t tag = sentry_value_get_by_key(eventTags, TCHAR_TO_ANSI(*key));
	if (sentry_value_is_null(tag))
	{
		return false;
	}

	value = FString(sentry_value_as_string(tag));

	return true;
}

void FGenericPlatformSentryEvent::RemoveTag(const FString& key)
{
	sentry_value_t eventTags = sentry_value_get_by_key(Event, "tags");
	if (sentry_value_is_null(eventTags))
	{
		return;
	}

	sentry_value_remove_by_key(eventTags, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentryEvent::SetTags(const TMap<FString, FString>& tags)
{
	sentry_value_set_by_key(Event, "tags", FGenericPlatformSentryConverters::StringMapToNative(tags));
}

TMap<FString, FString> FGenericPlatformSentryEvent::GetTags() const
{
	return FGenericPlatformSentryConverters::StringMapToUnreal(sentry_value_get_by_key(Event, "tags"));
}

void FGenericPlatformSentryEvent::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	sentry_value_t eventContexts = sentry_value_get_by_key(Event, "contexts");
	if (sentry_value_is_null(eventContexts))
	{
		eventContexts = sentry_value_new_object();

		sentry_value_set_by_key(eventContexts, TCHAR_TO_ANSI(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));

		sentry_value_set_by_key(Event, "contexts", eventContexts);
	}
	else
	{
		sentry_value_set_by_key(eventContexts, TCHAR_TO_ANSI(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));
	}
}

TMap<FString, FSentryVariant> FGenericPlatformSentryEvent::GetContext(const FString& key) const
{
	sentry_value_t eventContexts = sentry_value_get_by_key(Event, "contexts");
	if (sentry_value_is_null(eventContexts))
	{
		return TMap<FString, FSentryVariant>();
	}

	sentry_value_t context = sentry_value_get_by_key(eventContexts, TCHAR_TO_ANSI(*key));
	if (sentry_value_is_null(context))
	{
		return TMap<FString, FSentryVariant>();
	}

	const FSentryVariant& contextVariant = FGenericPlatformSentryConverters::VariantToUnreal(context);

	return contextVariant.GetValue<TMap<FString, FSentryVariant>>();
}

bool FGenericPlatformSentryEvent::TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const
{
	sentry_value_t eventContexts = sentry_value_get_by_key(Event, "contexts");
	if (sentry_value_is_null(eventContexts))
	{
		return false;
	}

	sentry_value_t context = sentry_value_get_by_key(eventContexts, TCHAR_TO_ANSI(*key));
	if (sentry_value_is_null(context))
	{
		return false;
	}

	const FSentryVariant& contextVariant = FGenericPlatformSentryConverters::VariantToUnreal(context);
	if (contextVariant.GetType() == ESentryVariantType::Empty)
	{
		return false;
	}

	value = contextVariant.GetValue<TMap<FString, FSentryVariant>>();

	return true;
}

void FGenericPlatformSentryEvent::RemoveContext(const FString& key)
{
	sentry_value_t eventContexts = sentry_value_get_by_key(Event, "contexts");
	if (sentry_value_is_null(eventContexts))
	{
		return;
	}

	sentry_value_remove_by_key(eventContexts, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentryEvent::SetExtra(const FString& key, const FSentryVariant& value)
{
	sentry_value_t eventExtra = sentry_value_get_by_key(Event, "extra");
	if (sentry_value_is_null(eventExtra))
	{
		sentry_value_set_by_key(Event, "extra", FGenericPlatformSentryConverters::VariantMapToNative({ { key, value } }));
	}
	else
	{
		sentry_value_set_by_key(eventExtra, TCHAR_TO_ANSI(*key), FGenericPlatformSentryConverters::VariantToNative(value));
	}
}

FSentryVariant FGenericPlatformSentryEvent::GetExtra(const FString& key) const
{
	sentry_value_t eventExtra = sentry_value_get_by_key(Event, "extra");
	if (sentry_value_is_null(eventExtra))
	{
		return FSentryVariant();
	}

	sentry_value_t extra = sentry_value_get_by_key(eventExtra, TCHAR_TO_ANSI(*key));
	return FGenericPlatformSentryConverters::VariantToUnreal(extra);
}

bool FGenericPlatformSentryEvent::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	sentry_value_t eventExtra = sentry_value_get_by_key(Event, "extra");
	if (sentry_value_is_null(eventExtra))
	{
		return false;
	}

	sentry_value_t extra = sentry_value_get_by_key(eventExtra, TCHAR_TO_ANSI(*key));
	if (sentry_value_is_null(extra))
	{
		return false;
	}

	value = FGenericPlatformSentryConverters::VariantToUnreal(extra);

	return true;
}

void FGenericPlatformSentryEvent::RemoveExtra(const FString& key)
{
	sentry_value_t eventExtra = sentry_value_get_by_key(Event, "extra");
	if (sentry_value_is_null(eventExtra))
	{
		return;
	}

	sentry_value_remove_by_key(eventExtra, TCHAR_TO_ANSI(*key));
}

void FGenericPlatformSentryEvent::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	sentry_value_set_by_key(Event, "extra", FGenericPlatformSentryConverters::VariantMapToNative(extras));
}

TMap<FString, FSentryVariant> FGenericPlatformSentryEvent::GetExtras() const
{
	sentry_value_t eventExtra = sentry_value_get_by_key(Event, "extra");
	if (sentry_value_is_null(eventExtra))
	{
		return TMap<FString, FSentryVariant>();
	}

	const FSentryVariant& extrasVariant = FGenericPlatformSentryConverters::VariantToUnreal(eventExtra);
	if (extrasVariant.GetType() == ESentryVariantType::Empty)
	{
		return TMap<FString, FSentryVariant>();
	}

	return extrasVariant.GetValue<TMap<FString, FSentryVariant>>();
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
