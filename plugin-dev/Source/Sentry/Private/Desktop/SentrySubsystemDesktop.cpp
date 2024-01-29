// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemDesktop.h"
#include "SentryEventDesktop.h"
#include "SentryBreadcrumbDesktop.h"
#include "SentryUserDesktop.h"
#include "SentryScopeDesktop.h"
#include "SentryTransactionDesktop.h"
#include "SentryTransactionContextDesktop.h"

#include "SentryDefines.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"
#include "SentryTransaction.h"
#include "SentryModule.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentryTransactionContext.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#include "CrashReporter/SentryCrashReporter.h"
#include "CrashReporter/SentryCrashContext.h"

#include "Transport/SentryTransport.h"

#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "HAL/FileManager.h"
#include "Runtime/Launch/Resources/Version.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

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
	SentrySubsystemDesktop* SentrySubsystem = static_cast<SentrySubsystemDesktop*>(closure);

	USentryEvent* EventToProcess = NewObject<USentryEvent>();
	EventToProcess->InitWithNativeImpl(MakeShareable(new SentryEventDesktop(event)));

	SentrySubsystem->GetCurrentScope()->Apply(EventToProcess);

	return SentrySubsystem->GetBeforeSendHandler()->HandleBeforeSend(EventToProcess, nullptr) ? event : sentry_value_new_null();
}

sentry_value_t HandleBeforeCrash(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
{
	SentrySubsystemDesktop* SentrySubsystem = static_cast<SentrySubsystemDesktop*>(closure);

	FSentryCrashContext::Get()->Apply(SentrySubsystem->GetCurrentScope());

	return HandleBeforeSend(event, nullptr, closure);
}

SentrySubsystemDesktop::SentrySubsystemDesktop()
	: beforeSend(nullptr)
	, crashReporter(MakeShareable(new SentryCrashReporter))
	, isEnabled(false)
	, isStackTraceEnabled(true)
{
}

void SentrySubsystemDesktop::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	beforeSend = beforeSendHandler;

	scopeStack.Push(MakeShareable(new SentryScopeDesktop()));

	sentry_options_t* options = sentry_options_new();

	if(settings->EnableAutoLogAttachment)
	{
		const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();

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

	if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
	{
		sentry_options_set_traces_sample_rate(options, settings->TracesSampleRate);
	}
	if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("The Native SDK doesn't currently support sampling functions"));
	}

#if PLATFORM_WINDOWS
	if(!FSentryModule::Get().IsMarketplaceVersion())
	{
		const FString HandlerPath = GetHandlerPath();

		if(!FPaths::FileExists(HandlerPath))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Crashpad executable couldn't be found so Breakpad will be used instead. "
				"Please make sure that the plugin was rebuilt to avoid initialization failure."));
		}

		sentry_options_set_handler_pathw(options, *HandlerPath);
	}
#elif PLATFORM_LINUX
	sentry_options_set_handler_path(options, TCHAR_TO_ANSI(*GetHandlerPath()));
#endif

#if PLATFORM_WINDOWS
	sentry_options_set_database_pathw(options, *GetDatabasePath());
#elif PLATFORM_LINUX
	sentry_options_set_database_path(options, TCHAR_TO_ANSI(*GetDatabasePath()));
#endif

	sentry_options_set_release(options, TCHAR_TO_ANSI(settings->OverrideReleaseName
		? *settings->Release
		: *settings->GetFormattedReleaseName()));

	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->Dsn));
	sentry_options_set_environment(options, TCHAR_TO_ANSI(*settings->Environment));
	sentry_options_set_logger(options, PrintVerboseLog, nullptr);
	sentry_options_set_debug(options, settings->Debug);
	sentry_options_set_auto_session_tracking(options, settings->EnableAutoSessionTracking);
	sentry_options_set_sample_rate(options, settings->SampleRate);
	sentry_options_set_max_breadcrumbs(options, settings->MaxBreadcrumbs);
	sentry_options_set_before_send(options, HandleBeforeSend, this);
	sentry_options_set_on_crash(options, HandleBeforeCrash, this);

#if PLATFORM_LINUX
	sentry_options_set_transport(options, FSentryTransport::Create());
#endif

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

	sentry_clear_crashed_last_run();

#if PLATFORM_WINDOWS && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	FPlatformMisc::SetCrashHandlingType(settings->EnableAutoCrashCapturing
		? ECrashHandlingType::Disabled
		: ECrashHandlingType::Default);
#endif

	isStackTraceEnabled = settings->AttachStacktrace;

	crashReporter->SetRelease(settings->Release);
	crashReporter->SetEnvironment(settings->Environment);
}

void SentrySubsystemDesktop::Close()
{
	isEnabled = false;

	sentry_close();

	scopeStack.Empty();
}

bool SentrySubsystemDesktop::IsEnabled()
{
	return isEnabled;
}

ESentryCrashedLastRun SentrySubsystemDesktop::IsCrashedLastRun()
{
	ESentryCrashedLastRun unrealIsCrashed = ESentryCrashedLastRun::NotEvaluated;

	switch (sentry_get_crashed_last_run())
	{
	case -1:
		unrealIsCrashed = ESentryCrashedLastRun::NotEvaluated;
		break;
	case 0:
		unrealIsCrashed = ESentryCrashedLastRun::NotCrashed;
		break;
	case 1:
		unrealIsCrashed = ESentryCrashedLastRun::Crashed;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown IsCrashedLastRun result. NotEvaluated will be returned."));
	}

	return unrealIsCrashed;
}

void SentrySubsystemDesktop::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	GetCurrentScope()->AddBreadcrumb(breadcrumb);
}

void SentrySubsystemDesktop::ClearBreadcrumbs()
{
	GetCurrentScope()->ClearBreadcrumbs();
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
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<SentryScopeDesktop> NewLocalScope = MakeShareable(new SentryScopeDesktop(*GetCurrentScope()));

	USentryScope* Scope = NewObject<USentryScope>();
	Scope->InitWithNativeImpl(NewLocalScope);

	onScopeConfigure.ExecuteIfBound(Scope);

	scopeStack.Push(NewLocalScope);
	USentryId* Id = CaptureMessage(message, level);
	scopeStack.Pop();

	return Id;
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
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<SentryScopeDesktop> NewLocalScope = MakeShareable(new SentryScopeDesktop(*GetCurrentScope()));

	USentryScope* Scope = NewObject<USentryScope>();
	Scope->InitWithNativeImpl(NewLocalScope);

	onScopeConfigure.ExecuteIfBound(Scope);

	scopeStack.Push(NewLocalScope);
	USentryId* Id = CaptureEvent(event);
	scopeStack.Pop();

	return Id;
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
	USentryScope* Scope = NewObject<USentryScope>();
	Scope->InitWithNativeImpl(GetCurrentScope());

	onConfigureScope.ExecuteIfBound(Scope);
}

void SentrySubsystemDesktop::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	GetCurrentScope()->SetContext(key, values);

	crashReporter->SetContext(key, values);
}

void SentrySubsystemDesktop::SetTag(const FString& key, const FString& value)
{
	GetCurrentScope()->SetTagValue(key, value);

	crashReporter->SetTag(key, value);
}

void SentrySubsystemDesktop::RemoveTag(const FString& key)
{
	GetCurrentScope()->RemoveTag(key);

	crashReporter->RemoveTag(key);
}

void SentrySubsystemDesktop::SetLevel(ESentryLevel level)
{
	GetCurrentScope()->SetLevel(level);
}

void SentrySubsystemDesktop::StartSession()
{
	sentry_start_session();
}

void SentrySubsystemDesktop::EndSession()
{
	sentry_end_session();
}

USentryTransaction* SentrySubsystemDesktop::StartTransaction(const FString& name, const FString& operation)
{
	sentry_transaction_context_t* transactionContext = sentry_transaction_context_new(TCHAR_TO_ANSI(*name), TCHAR_TO_ANSI(*operation));

	sentry_transaction_t* nativeTransaction = sentry_transaction_start(transactionContext, sentry_value_new_null());

	return SentryConvertorsDesktop::SentryTransactionToUnreal(nativeTransaction);
}

USentryTransaction* SentrySubsystemDesktop::StartTransactionWithContext(USentryTransactionContext* context)
{
	TSharedPtr<SentryTransactionContextDesktop> transactionContextDesktop = StaticCastSharedPtr<SentryTransactionContextDesktop>(context->GetNativeImpl());

	sentry_transaction_t* nativeTransaction = sentry_transaction_start(transactionContextDesktop->GetNativeObject(), sentry_value_new_null());

	return SentryConvertorsDesktop::SentryTransactionToUnreal(nativeTransaction);
}

USentryTransaction* SentrySubsystemDesktop::StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Transaction options currently not supported on desktop."));
	return StartTransactionWithContext(context);
}

USentryBeforeSendHandler* SentrySubsystemDesktop::GetBeforeSendHandler()
{
	return beforeSend;
}

TSharedPtr<SentryScopeDesktop> SentrySubsystemDesktop::GetCurrentScope()
{
	if(scopeStack.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Scope stack is empty."));
		return nullptr;
	}

	return scopeStack.Top();
}

FString SentrySubsystemDesktop::GetHandlerPath() const
{
#if PLATFORM_WINDOWS
	const FString HandlerExecutableName = TEXT("crashpad_handler.exe");
#elif PLATFORM_LINUX
	const FString HandlerExecutableName = TEXT("crashpad_handler");
#endif

	const FString HandlerPath = FPaths::Combine(FSentryModule::Get().GetBinariesPath(), HandlerExecutableName);
	const FString HandlerFullPath = FPaths::ConvertRelativePathToFull(HandlerPath);

	return HandlerFullPath;
}

FString SentrySubsystemDesktop::GetDatabasePath() const
{
	const FString DatabasePath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".sentry-native"));
	const FString DatabaseFullPath = FPaths::ConvertRelativePathToFull(DatabasePath);

	return DatabaseFullPath;
}

#endif
