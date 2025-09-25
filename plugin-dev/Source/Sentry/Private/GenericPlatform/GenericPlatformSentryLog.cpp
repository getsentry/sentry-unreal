// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryLog.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryLog::FGenericPlatformSentryLog()
{
	Log = sentry_value_new_object();
	sentry_value_set_by_key(Log, "level", sentry_value_new_string("info"));
}

FGenericPlatformSentryLog::FGenericPlatformSentryLog(sentry_value_t log)
{
	Log = log;
}

FGenericPlatformSentryLog::FGenericPlatformSentryLog(const FString& message, ESentryLevel level, const FString& category)
{
	Log = sentry_value_new_object();
	SetMessage(message);
	SetLevel(level);
}

sentry_value_t FGenericPlatformSentryLog::GetNativeObject()
{
	return Log;
}

void FGenericPlatformSentryLog::SetMessage(const FString& message)
{
	sentry_value_set_by_key(Log, "message", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
}

FString FGenericPlatformSentryLog::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(Log, "message");
	return FString(sentry_value_as_string(message));
}

void FGenericPlatformSentryLog::SetLevel(ESentryLevel level)
{
	const char* levelStr = nullptr;
	switch (level)
	{
	case ESentryLevel::Fatal:
		levelStr = "fatal";
		break;
	case ESentryLevel::Error:
		levelStr = "error";
		break;
	case ESentryLevel::Warning:
		levelStr = "warning";
		break;
	case ESentryLevel::Info:
		levelStr = "info";
		break;
	case ESentryLevel::Debug:
		levelStr = "debug";
		break;
	}
	sentry_value_set_by_key(Log, "level", sentry_value_new_string(levelStr));
}

ESentryLevel FGenericPlatformSentryLog::GetLevel() const
{
	sentry_value_t levelValue = sentry_value_get_by_key(Log, "level");
	const char* levelStr = sentry_value_as_string(levelValue);

	if (FCStringAnsi::Strcmp(levelStr, "fatal") == 0)
		return ESentryLevel::Fatal;
	else if (FCStringAnsi::Strcmp(levelStr, "error") == 0)
		return ESentryLevel::Error;
	else if (FCStringAnsi::Strcmp(levelStr, "warning") == 0)
		return ESentryLevel::Warning;
	else if (FCStringAnsi::Strcmp(levelStr, "info") == 0)
		return ESentryLevel::Info;
	else if (FCStringAnsi::Strcmp(levelStr, "debug") == 0)
		return ESentryLevel::Debug;
	else
		return ESentryLevel::Info; // default
}

#endif // USE_SENTRY_NATIVE