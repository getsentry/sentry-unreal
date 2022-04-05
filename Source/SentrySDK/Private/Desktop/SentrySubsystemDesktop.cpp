// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemDesktop.h"

#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryUserFeedback.h"
#include "SentryModule.h"

#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "sentry.h"
#include "Windows/HideWindowsPlatformTypes.h"
#elif PLATFORM_MAC
#include "sentry.h"
#endif

void SentrySubsystemDesktop::InitWithSettings(const USentrySettings* settings)
{
#if PLATFORM_WINDOWS
	const FString HandlerExecutableName = TEXT("crashpad_handler.exe");
#elif PLATFORM_MAC
	const FString HandlerExecutableName = TEXT("crashpad_handler");
#endif

	const FString HandlerPath = FPaths::Combine(FSentryModule::Get().GetBinariesPath(), HandlerExecutableName);

	sentry_options_t* options = sentry_options_new();
	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->DsnUrl));
	sentry_options_set_handler_path(options, TCHAR_TO_ANSI(*HandlerPath));
	sentry_init(options);
}

void SentrySubsystemDesktop::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	
}

USentryId* SentrySubsystemDesktop::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(SENTRY_LEVEL_INFO, NULL, TCHAR_TO_ANSI(*message));
	sentry_uuid_t id = sentry_capture_event(sentryEvent);

	return nullptr;
}

USentryId* SentrySubsystemDesktop::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	return nullptr;
}

USentryId* SentrySubsystemDesktop::CaptureEvent(USentryEvent* event)
{
	return nullptr;
}

USentryId* SentrySubsystemDesktop::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	return nullptr;
}

void SentrySubsystemDesktop::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
}
