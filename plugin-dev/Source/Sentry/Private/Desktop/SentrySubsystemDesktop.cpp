// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemDesktop.h"
#include "SentryEventDesktop.h"
#include "SentryBreadcrumbDesktop.h"
#include "SentryUserDesktop.h"
#include "SentryDefines.h"

#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"
#include "SentryModule.h"

#include "Infrastructure/SentryConvertorsDesktop.h"
#include "CrashReporter/SentryCrashReporter.h"

#include "Misc/Paths.h"

void PrintVerboseLog(sentry_level_t level, const char *message, va_list args, void *userdata)
{
	char buffer[512];
	vsnprintf(buffer, 512, message, args);

	UE_LOG(LogSentrySdk, Log, TEXT("%s"), *FString(buffer));
}

SentrySubsystemDesktop::SentrySubsystemDesktop()
{
	crashReporter = MakeShareable(new SentryCrashReporter);
}

void SentrySubsystemDesktop::InitWithSettings(const USentrySettings* settings)
{
#if PLATFORM_WINDOWS
	const FString HandlerExecutableName = TEXT("crashpad_handler.exe");
#elif PLATFORM_MAC || PLATFORM_LINUX
	const FString HandlerExecutableName = TEXT("crashpad_handler");
#endif

	const FString HandlerPath = FPaths::Combine(FSentryModule::Get().GetBinariesPath(), HandlerExecutableName);

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad handler doesn't exist."));
		return;
	}

	sentry_options_t* options = sentry_options_new();
	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->DsnUrl));
	sentry_options_set_release(options, TCHAR_TO_ANSI(*settings->Release));
	sentry_options_set_handler_path(options, TCHAR_TO_ANSI(*HandlerPath));
	sentry_options_set_logger(options, PrintVerboseLog, nullptr);
	sentry_options_set_debug(options, settings->EnableVerboseLogging);
	sentry_init(options);

	crashReporter->SetRelease(settings->Release);
}

void SentrySubsystemDesktop::Close()
{
	sentry_close();
}

void SentrySubsystemDesktop::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbDesktop> breadcrumbDesktop = StaticCastSharedPtr<SentryBreadcrumbDesktop>(breadcrumb->GetNativeImpl());

	sentry_add_breadcrumb(breadcrumbDesktop->GetNativeObject());
}

void SentrySubsystemDesktop::ClearBreadcrumbs()
{
	UE_LOG(LogSentrySdk, Log, TEXT("CaptureMessageWithScope method is not supported for the current platform."));
}

USentryId* SentrySubsystemDesktop::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(SentryConvertorsDesktop::SentryLevelToNative(level), nullptr, TCHAR_TO_ANSI(*message));
	sentry_uuid_t id = sentry_capture_event(sentryEvent);
	return SentryConvertorsDesktop::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemDesktop::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	UE_LOG(LogSentrySdk, Log, TEXT("CaptureMessageWithScope method is not supported for the current platform."));
	return nullptr;
}

USentryId* SentrySubsystemDesktop::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventDesktop> eventDesktop = StaticCastSharedPtr<SentryEventDesktop>(event->GetNativeImpl());

	sentry_value_t nativeEvent = eventDesktop->GetNativeObject();
	sentry_uuid_t id = sentry_capture_event(nativeEvent);

	return SentryConvertorsDesktop::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemDesktop::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	UE_LOG(LogSentrySdk, Log, TEXT("CaptureEventWithScope method is not supported for the current platform."));
	return nullptr;
}

void SentrySubsystemDesktop::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	UE_LOG(LogSentrySdk, Log, TEXT("CaptureUserFeedback method is not supported for the current platform."));
}

void SentrySubsystemDesktop::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserDesktop> userDesktop = StaticCastSharedPtr<SentryUserDesktop>(user->GetNativeImpl());
	sentry_set_user(userDesktop->GetNativeObject());

	crashReporter->SetUser(user);
}

void SentrySubsystemDesktop::RemoveUser()
{
	sentry_remove_user();

	crashReporter->RemoveUser();
}

void SentrySubsystemDesktop::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	UE_LOG(LogSentrySdk, Log, TEXT("CaptureUserFeedback method is not supported for the current platform."));
}

void SentrySubsystemDesktop::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	sentry_set_context(TCHAR_TO_ANSI(*key), SentryConvertorsDesktop::StringMapToNative(values));

	crashReporter->SetContext(key, values);
}

void SentrySubsystemDesktop::SetTag(const FString& key, const FString& value)
{
	sentry_set_tag(TCHAR_TO_ANSI(*key), TCHAR_TO_ANSI(*value));

	crashReporter->SetTag(key, value);
}

void SentrySubsystemDesktop::RemoveTag(const FString& key)
{
	sentry_remove_tag(TCHAR_TO_ANSI(*key));

	crashReporter->RemoveTag(key);
}

void SentrySubsystemDesktop::SetLevel(ESentryLevel level)
{
	sentry_set_level(SentryConvertorsDesktop::SentryLevelToNative(level));
}