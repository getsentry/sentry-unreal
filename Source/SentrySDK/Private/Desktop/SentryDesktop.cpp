// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryDesktop.h"

#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryDefines.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "sentry.h"
#include "Windows/HideWindowsPlatformTypes.h"
#elif PLATFORM_MAC
#include "sentry.h"
#endif

void SentryDesktop::InitWithSettings(const USentrySettings* settings)
{
	sentry_options_t* options = sentry_options_new();
	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->DsnUrl));
	sentry_options_set_debug(options, 1);
	sentry_init(options);
	sentry_start_session();

	UE_LOG(LogSentrySdk, Warning, TEXT("SENTRY INITIALIZED"));
}

void SentryDesktop::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	
}

FString SentryDesktop::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(SENTRY_LEVEL_INFO, NULL, TCHAR_TO_ANSI(*message));
	sentry_uuid_t id = sentry_capture_event(sentryEvent);

	UE_LOG(LogSentrySdk, Warning, TEXT("SENTRY CaptureMessage"));
	return FString();
}

FString SentryDesktop::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	return FString();
}

FString SentryDesktop::CaptureEvent(USentryEvent* event)
{
	return FString();
}

FString SentryDesktop::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{

	return FString();
}
