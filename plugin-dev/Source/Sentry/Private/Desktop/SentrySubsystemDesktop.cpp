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
#include "SentryBeforeSendHandler.h"

#include "Infrastructure/SentryConvertorsDesktop.h"
#include "CrashReporter/SentryCrashReporter.h"
#include "Transport/SentryTransport.h"

#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Launch/Resources/Version.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#endif

#if USE_SENTRY_NATIVE

void PrintVerboseLog(sentry_level_t level, const char *message, va_list args, void *userdata)
{
	char buffer[512];
	vsnprintf(buffer, 512, message, args);

	UE_LOG(LogSentrySdk, Log, TEXT("%s"), *FString(buffer));
}

sentry_value_t HandleBeforeSend(sentry_value_t event, void *hint, void *closure)
{
	USentryBeforeSendHandler* handler = static_cast<USentryBeforeSendHandler*>(closure);

	USentryEvent* EventToProcess = NewObject<USentryEvent>();
	EventToProcess->InitWithNativeImpl(MakeShareable(new SentryEventDesktop(event)));

	return handler->HandleBeforeSend(EventToProcess, nullptr) ? event : sentry_value_new_null();
}

SentrySubsystemDesktop::SentrySubsystemDesktop()
	: crashReporter(MakeShareable(new SentryCrashReporter))
	, isEnabled(false)
	, isStackTraceEnabled(true)
{
}

void SentrySubsystemDesktop::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler)
{
#if PLATFORM_WINDOWS
	const FString HandlerExecutableName = TEXT("crashpad_handler.exe");
#elif PLATFORM_LINUX
	const FString HandlerExecutableName = TEXT("crashpad_handler");
#endif

	const FString HandlerPath = FPaths::Combine(FSentryModule::Get().GetBinariesPath(), HandlerExecutableName);
	const FString DatabasePath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".sentry-native"));

	const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();

	sentry_options_t* options = sentry_options_new();

	if(settings->EnableAutoLogAttachment)
	{
#if PLATFORM_WINDOWS
		sentry_options_add_attachmentw(options, *FPaths::ConvertRelativePathToFull(LogFilePath));
#elif PLATFORM_LINUX
		sentry_options_add_attachment(options, TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(LogFilePath)));
#endif
	}

	if(settings->UseProxy)
	{
		sentry_options_set_http_proxy(options, TCHAR_TO_ANSI(*settings->ProxyUrl));
	}

#if PLATFORM_WINDOWS
	sentry_options_set_handler_pathw(options, *FPaths::ConvertRelativePathToFull(HandlerPath));
	sentry_options_set_database_pathw(options, *FPaths::ConvertRelativePathToFull(DatabasePath));
#elif PLATFORM_LINUX
	sentry_options_set_handler_path(options, TCHAR_TO_ANSI(*FPaths::ConvertRelativePathToFull(HandlerPath)));
	sentry_options_set_database_path(options, TCHAR_TO_ANSI(*FPaths::ConvertRelativePathToFull(DatabasePath)));
#endif

	sentry_options_set_release(options, TCHAR_TO_ANSI(settings->OverrideReleaseName
		? *settings->Release
		: *settings->GetFormattedReleaseName()));

	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->DsnUrl));
	sentry_options_set_environment(options, TCHAR_TO_ANSI(*settings->Environment));
	sentry_options_set_logger(options, PrintVerboseLog, nullptr);
	sentry_options_set_debug(options, settings->EnableVerboseLogging);
	sentry_options_set_auto_session_tracking(options, settings->EnableAutoSessionTracking);
	sentry_options_set_before_send(options, HandleBeforeSend, beforeSendHandler);

#if PLATFORM_LINUX
	sentry_options_set_transport(options, FSentryTransport::Create());
#endif

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

#if PLATFORM_WINDOWS && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	if(settings->EnableAutoCrashCapturing)
	{
		FPlatformMisc::SetCrashHandlingType(ECrashHandlingType::Disabled);
	}
#endif

	isStackTraceEnabled = settings->EnableStackTrace;

	crashReporter->SetRelease(settings->Release);
	crashReporter->SetEnvironment(settings->Environment);
}

void SentrySubsystemDesktop::Close()
{
	isEnabled = false;

	sentry_close();
}

bool SentrySubsystemDesktop::IsEnabled()
{
	return isEnabled;
}

void SentrySubsystemDesktop::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbDesktop> breadcrumbDesktop = StaticCastSharedPtr<SentryBreadcrumbDesktop>(breadcrumb->GetNativeImpl());

	sentry_add_breadcrumb(breadcrumbDesktop->GetNativeObject());
}

void SentrySubsystemDesktop::ClearBreadcrumbs()
{
	UE_LOG(LogSentrySdk, Log, TEXT("ClearBreadcrumbs method is not supported for the current platform."));
}

USentryId* SentrySubsystemDesktop::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(SentryConvertorsDesktop::SentryLevelToNative(level), nullptr, TCHAR_TO_ANSI(*message));

	if(isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(sentryEvent, nullptr, 0);
	}

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

	if(isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

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

void SentrySubsystemDesktop::StartSession()
{
	sentry_start_session();
}

void SentrySubsystemDesktop::EndSession()
{
	sentry_end_session();
}

#endif
