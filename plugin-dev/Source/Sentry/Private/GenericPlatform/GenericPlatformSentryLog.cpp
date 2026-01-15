// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryLog.h"

#if USE_SENTRY_NATIVE

#include "Infrastructure/GenericPlatformSentryConverters.h"

FGenericPlatformSentryLog::FGenericPlatformSentryLog()
{
	Log = sentry_value_new_object();
	sentry_value_set_by_key(Log, "level", sentry_value_new_string("info"));
}

FGenericPlatformSentryLog::FGenericPlatformSentryLog(sentry_value_t log)
{
	Log = log;
}

FGenericPlatformSentryLog::FGenericPlatformSentryLog(const FString& body, ESentryLevel level)
{
	Log = sentry_value_new_object();
	SetBody(body);
	SetLevel(level);
}

sentry_value_t FGenericPlatformSentryLog::GetNativeObject()
{
	return Log;
}

void FGenericPlatformSentryLog::SetBody(const FString& body)
{
	sentry_value_set_by_key(Log, "body", sentry_value_new_string(TCHAR_TO_UTF8(*body)));
}

FString FGenericPlatformSentryLog::GetBody() const
{
	sentry_value_t body = sentry_value_get_by_key(Log, "body");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(body)));
}

void FGenericPlatformSentryLog::SetLevel(ESentryLevel level)
{
	const char* levelStr = "debug"; // Default to "debug" to avoid nullptr
	switch (level)
	{
	case ESentryLevel::Fatal:
		levelStr = "fatal";
		break;
	case ESentryLevel::Error:
		levelStr = "error";
		break;
	case ESentryLevel::Warning:
		levelStr = "warn";
		break;
	case ESentryLevel::Info:
		levelStr = "info";
		break;
	case ESentryLevel::Debug:
		levelStr = "debug";
		break;
	default:
		// Use default "debug" level for unhandled cases
		break;
	}
	sentry_value_set_by_key(Log, "level", sentry_value_new_string(levelStr));
}

ESentryLevel FGenericPlatformSentryLog::GetLevel() const
{
	sentry_value_t levelValue = sentry_value_get_by_key(Log, "level");
	const char* levelStr = sentry_value_as_string(levelValue);

	if (!levelStr)
	{
		return ESentryLevel::Debug; // default
	}

	if (FCStringAnsi::Strcmp(levelStr, "fatal") == 0)
		return ESentryLevel::Fatal;
	else if (FCStringAnsi::Strcmp(levelStr, "error") == 0)
		return ESentryLevel::Error;
	else if (FCStringAnsi::Strcmp(levelStr, "warn") == 0)
		return ESentryLevel::Warning;
	else if (FCStringAnsi::Strcmp(levelStr, "info") == 0)
		return ESentryLevel::Info;
	else if (FCStringAnsi::Strcmp(levelStr, "debug") == 0)
		return ESentryLevel::Debug;
	else if (FCStringAnsi::Strcmp(levelStr, "trace") == 0) // map trace to debug
		return ESentryLevel::Debug;
	else
		return ESentryLevel::Debug; // default
}

void FGenericPlatformSentryLog::SetAttribute(const FString& key, const FSentryVariant& value)
{
	sentry_value_t attributes = sentry_value_get_by_key(Log, "attributes");
	if (sentry_value_is_null(attributes))
	{
		attributes = sentry_value_new_object();
		sentry_value_set_by_key(Log, "attributes", attributes);
	}

	sentry_value_t attribute = FGenericPlatformSentryConverters::VariantToAttributeNative(value);
	sentry_value_set_by_key(attributes, TCHAR_TO_UTF8(*key), attribute);
}

FSentryVariant FGenericPlatformSentryLog::GetAttribute(const FString& key) const
{
	sentry_value_t attributes = sentry_value_get_by_key(Log, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return FSentryVariant();
	}

	sentry_value_t attribute = sentry_value_get_by_key(attributes, TCHAR_TO_UTF8(*key));
	if (sentry_value_is_null(attribute))
	{
		return FSentryVariant();
	}

	sentry_value_t attributeValue = sentry_value_get_by_key(attribute, "value");
	return FGenericPlatformSentryConverters::VariantToUnreal(attributeValue);
}

bool FGenericPlatformSentryLog::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	sentry_value_t attributes = sentry_value_get_by_key(Log, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return false;
	}

	sentry_value_t attribute = sentry_value_get_by_key(attributes, TCHAR_TO_UTF8(*key));
	if (sentry_value_is_null(attribute))
	{
		return false;
	}

	sentry_value_t attributeValue = sentry_value_get_by_key(attribute, "value");
	if (sentry_value_is_null(attributeValue))
	{
		return false;
	}

	value = FGenericPlatformSentryConverters::VariantToUnreal(attributeValue);

	return true;
}

void FGenericPlatformSentryLog::RemoveAttribute(const FString& key)
{
	sentry_value_t attributes = sentry_value_get_by_key(Log, "attributes");
	if (sentry_value_is_null(attributes))
	{
		return;
	}

	sentry_value_remove_by_key(attributes, TCHAR_TO_UTF8(*key));
}

void FGenericPlatformSentryLog::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	for (const auto& pair : attributes)
	{
		SetAttribute(pair.Key, pair.Value);
	}
}

#endif // USE_SENTRY_NATIVE