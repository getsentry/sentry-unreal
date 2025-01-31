// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySubsystem.h"
#include "GenericPlatformSentryEvent.h"
#include "GenericPlatformSentryBreadcrumb.h"
#include "GenericPlatformSentryUser.h"
#include "GenericPlatformSentryUserFeedback.h"
#include "GenericPlatformSentryScope.h"
#include "GenericPlatformSentryTransaction.h"
#include "GenericPlatformSentryTransactionContext.h"
#include "GenericPlatformSentryId.h"

#include "SentryDefines.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryModule.h"
#include "SentryBeforeSendHandler.h"

#include "SentryTraceSampler.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryLogUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashReporter.h"
#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashContext.h"

#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"
#include "Misc/EngineVersionComparison.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/ExceptionHandling.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

extern CORE_API bool GIsGPUCrashed;

#if USE_SENTRY_NATIVE

void PrintVerboseLog(sentry_level_t level, const char* message, va_list args, void* userdata)
{
	char buffer[512];
	vsnprintf(buffer, 512, message, args);

	FString MessageBuf = FString(buffer);

	// The WER (Windows Error Reporting) module (crashpad_wer.dll) can't be distributed along with other Sentry binaries
	// within the plugin package due to some UE Marketplace restrictions. Its absence doesn't affect crash capturing
	// and the corresponding warning can be disregarded
	if (MessageBuf.Equals(TEXT("crashpad WER handler module not found")))
	{
		return;
	}

#if !NO_LOGGING
	const FName SentryCategoryName(LogSentrySdk.GetCategoryName());
#else
	const FName SentryCategoryName(TEXT("LogSentrySdk"));
#endif

	GLog->CategorizedLogf(SentryCategoryName, FGenericPlatformSentryConverters::SentryLevelToLogVerbosity(level), TEXT("%s"), *MessageBuf);
}

/* static */ sentry_value_t FGenericPlatformSentrySubsystem::HandleBeforeSend(sentry_value_t event, void* hint, void* closure)
{
	if (closure)
	{
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnBeforeSend(event, hint, closure);
	}
	else
	{
		return event;
	}
}

/* static */ sentry_value_t FGenericPlatformSentrySubsystem::HandleOnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	if (closure)
	{
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnCrash(uctx, event, closure);
	}
	else
	{
		return event;
	}
}

sentry_value_t FGenericPlatformSentrySubsystem::OnBeforeSend(sentry_value_t event, void* hint, void* closure)
{
	if (!closure || this != closure)
	{
		return event;
	}

	TSharedPtr<FGenericPlatformSentryEvent> Event = MakeShareable(new FGenericPlatformSentryEvent(event));

	GetCurrentScope()->Apply(Event);

	FGCScopeGuard GCScopeGuard;

	USentryEvent* EventToProcess = USentryEvent::Create(Event);

	USentryEvent* ProcessedEvent = EventToProcess;
	if (!FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		// Executing UFUNCTION is allowed only when not post-loading
		ProcessedEvent = GetBeforeSendHandler()->HandleBeforeSend(EventToProcess, nullptr);
	}

	return ProcessedEvent ? event : sentry_value_new_null();
}

sentry_value_t FGenericPlatformSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	if (!closure || this != closure)
	{
		return event;
	}

	TryCaptureScreenshot();

	if (GIsGPUCrashed)
	{
		IFileManager::Get().Copy(*GetGpuDumpBackupPath(), *SentryFileUtils::GetGpuDumpPath());
	}

	FGenericPlatformSentryCrashContext::Get()->Apply(GetCurrentScope());

	TSharedPtr<FGenericPlatformSentryEvent> Event = MakeShareable(new FGenericPlatformSentryEvent(event, true));

	GetCurrentScope()->Apply(Event);

	if (!IsGarbageCollecting())
	{
		USentryEvent* EventToProcess = USentryEvent::Create(Event);

		USentryEvent* ProcessedEvent = EventToProcess;
		if (!FUObjectThreadContext::Get().IsRoutingPostLoad)
		{
			// Executing UFUNCTION is allowed only when not post-loading
			ProcessedEvent = GetBeforeSendHandler()->HandleBeforeSend(EventToProcess, nullptr);
		}

		return ProcessedEvent ? event : sentry_value_new_null();
	}
	else
	{
		// If crash occurred during garbage collection we can't just obtain a GC lock like with normal events
		// since there is no guarantee it will be ever freed. In this case crash event will be reported
		// without calling a `beforeSend` handler.
		return event;
	}
}

FGenericPlatformSentrySubsystem::FGenericPlatformSentrySubsystem()
	: beforeSend(nullptr)
	, crashReporter(MakeShareable(new FGenericPlatformSentryCrashReporter))
	, isEnabled(false)
	, isStackTraceEnabled(true)
	, isScreenshotAttachmentEnabled(false)
{
}

void FGenericPlatformSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	beforeSend = beforeSendHandler;

	scopeStack.Push(MakeShareable(new FGenericPlatformSentryScope()));

	sentry_options_t* options = sentry_options_new();

	if (settings->EnableAutoLogAttachment)
	{
		ConfigureLogFileAttachment(options);
	}

	switch (settings->DatabaseLocation)
	{
	case ESentryDatabaseLocation::ProjectDirectory:
		databaseParentPath = FPaths::ProjectDir();
		break;
	case ESentryDatabaseLocation::ProjectUserDirectory:
		databaseParentPath = FPaths::ProjectUserDir();
		break;
	}

	if (databaseParentPath.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry database location. Falling back to FPaths::ProjectUserDir()."));
		databaseParentPath = FPaths::ProjectUserDir();
	}

	if (settings->AttachScreenshot)
	{
		isScreenshotAttachmentEnabled = true;

		ConfigureScreenshotAttachment(options);
	}

	if (settings->AttachGpuDump)
	{
		ConfigureGpuDumpAttachment(options);
	}

	if (settings->UseProxy)
	{
		sentry_options_set_http_proxy(options, TCHAR_TO_ANSI(*settings->ProxyUrl));
	}

	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
	{
		sentry_options_set_traces_sample_rate(options, settings->TracesSampleRate);
	}
	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("The Native SDK doesn't currently support sampling functions"));
	}

	ConfigureHandlerPath(options);
	ConfigureDatabasePath(options);

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
	sentry_options_set_on_crash(options, HandleOnCrash, this);
	sentry_options_set_shutdown_timeout(options, 3000);
	// TODO: Remove magic number when https://github.com/getsentry/sentry-native/issues/1131 is resolved
	sentry_options_set_max_spans(options, 1000);

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

	sentry_clear_crashed_last_run();

	isStackTraceEnabled = settings->AttachStacktrace;

	crashReporter->SetRelease(settings->Release);
	crashReporter->SetEnvironment(settings->Environment);
}

void FGenericPlatformSentrySubsystem::Close()
{
	isEnabled = false;

	sentry_close();

	scopeStack.Empty();
}

bool FGenericPlatformSentrySubsystem::IsEnabled()
{
	return isEnabled;
}

ESentryCrashedLastRun FGenericPlatformSentrySubsystem::IsCrashedLastRun()
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

void FGenericPlatformSentrySubsystem::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	GetCurrentScope()->AddBreadcrumb(breadcrumb);
}

void FGenericPlatformSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	TSharedPtr<FGenericPlatformSentryBreadcrumb> Breadcrumb = MakeShareable(new FGenericPlatformSentryBreadcrumb());
	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	GetCurrentScope()->AddBreadcrumb(Breadcrumb);
}

void FGenericPlatformSentrySubsystem::ClearBreadcrumbs()
{
	GetCurrentScope()->ClearBreadcrumbs();
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(FGenericPlatformSentryConverters::SentryLevelToNative(level), nullptr, TCHAR_TO_UTF8(*message));

	if (isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(sentryEvent, nullptr, 0);
	}

	sentry_uuid_t id = sentry_capture_event(sentryEvent);
	return MakeShareable(new FGenericPlatformSentryId(id));
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<FGenericPlatformSentryScope> NewLocalScope = MakeShareable(new FGenericPlatformSentryScope(*GetCurrentScope()));

	onScopeConfigure.ExecuteIfBound(NewLocalScope);

	scopeStack.Push(NewLocalScope);
	TSharedPtr<ISentryId> Id = CaptureMessage(message, level);
	scopeStack.Pop();

	return Id;
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<FGenericPlatformSentryEvent> Event = StaticCastSharedPtr<FGenericPlatformSentryEvent>(event);

	sentry_value_t nativeEvent = Event->GetNativeObject();

	if (isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

	sentry_uuid_t id = sentry_capture_event(nativeEvent);
	return MakeShareable(new FGenericPlatformSentryId(id));
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure)
{
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<FGenericPlatformSentryScope> NewLocalScope = MakeShareable(new FGenericPlatformSentryScope(*GetCurrentScope()));

	onScopeConfigure.ExecuteIfBound(NewLocalScope);

	scopeStack.Push(NewLocalScope);
	TSharedPtr<ISentryId> Id = CaptureEvent(event);
	scopeStack.Pop();

	return Id;
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureException(const FString& type, const FString& message, int32 framesToSkip)
{
	sentry_value_t exceptionEvent = sentry_value_new_event();

	auto StackFrames = FGenericPlatformStackWalk::GetStack(framesToSkip);
	sentry_value_set_by_key(exceptionEvent, "stacktrace", FGenericPlatformSentryConverters::CallstackToNative(StackFrames));

	sentry_value_t nativeException = sentry_value_new_exception(TCHAR_TO_ANSI(*type), TCHAR_TO_ANSI(*message));
	sentry_event_add_exception(exceptionEvent, nativeException);

	sentry_uuid_t id = sentry_capture_event(exceptionEvent);
	return MakeShareable(new FGenericPlatformSentryId(id));
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureAssertion(const FString& type, const FString& message)
{
	int32 framesToSkip = GetAssertionFramesToSkip();

	SentryLogUtils::LogStackTrace(*message, ELogVerbosity::Error, framesToSkip);

	return CaptureException(type, message, GetAssertionFramesToSkip());
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	return CaptureException(type, message, GetEnsureFramesToSkip());
}

void FGenericPlatformSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> InUserFeedback)
{
	TSharedPtr<FGenericPlatformSentryUserFeedback> userFeedback = StaticCastSharedPtr<FGenericPlatformSentryUserFeedback>(InUserFeedback);
	sentry_capture_user_feedback(userFeedback->GetNativeObject());
}

void FGenericPlatformSentrySubsystem::SetUser(TSharedPtr<ISentryUser> InUser)
{
	TSharedPtr<FGenericPlatformSentryUser> user = StaticCastSharedPtr<FGenericPlatformSentryUser>(InUser);
	sentry_set_user(user->GetNativeObject());

	crashReporter->SetUser(user);
}

void FGenericPlatformSentrySubsystem::RemoveUser()
{
	sentry_remove_user();

	crashReporter->RemoveUser();
}

void FGenericPlatformSentrySubsystem::ConfigureScope(const FSentryScopeDelegate& onConfigureScope)
{
	onConfigureScope.ExecuteIfBound(GetCurrentScope());
}

void FGenericPlatformSentrySubsystem::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	GetCurrentScope()->SetContext(key, values);

	crashReporter->SetContext(key, values);
}

void FGenericPlatformSentrySubsystem::SetTag(const FString& key, const FString& value)
{
	GetCurrentScope()->SetTagValue(key, value);

	crashReporter->SetTag(key, value);
}

void FGenericPlatformSentrySubsystem::RemoveTag(const FString& key)
{
	GetCurrentScope()->RemoveTag(key);

	crashReporter->RemoveTag(key);
}

void FGenericPlatformSentrySubsystem::SetLevel(ESentryLevel level)
{
	GetCurrentScope()->SetLevel(level);
}

void FGenericPlatformSentrySubsystem::StartSession()
{
	sentry_start_session();
}

void FGenericPlatformSentrySubsystem::EndSession()
{
	sentry_end_session();
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransaction(const FString& name, const FString& operation)
{
	TSharedPtr<ISentryTransactionContext> transactionContext = MakeShareable(new FGenericPlatformSentryTransactionContext(name, operation));

	return StartTransactionWithContext(transactionContext);
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	if (sentry_transaction_t* nativeTransaction = sentry_transaction_start(context->GetNativeObject(), sentry_value_new_null()))
	{
		return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	if (sentry_transaction_t* nativeTransaction = sentry_transaction_start_ts(context->GetNativeObject(), sentry_value_new_null(), timestamp))
	{
		return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
	}
	else
	{
		return nullptr;
	}
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Transaction options currently not supported (and therefore ignored) on generic platform."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransactionContext> FGenericPlatformSentrySubsystem::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	TSharedPtr<FGenericPlatformSentryTransactionContext> transactionContext = MakeShareable(new FGenericPlatformSentryTransactionContext(TEXT("<unlabeled transaction>"), TEXT("default")));

	sentry_transaction_context_update_from_header(transactionContext->GetNativeObject(), "sentry-trace", TCHAR_TO_ANSI(*sentryTrace));

	// currently `sentry-native` doesn't have API for `sentry_transaction_context_t` to set `baggageHeaders`

	return transactionContext;
}

USentryBeforeSendHandler* FGenericPlatformSentrySubsystem::GetBeforeSendHandler()
{
	return beforeSend;
}

void FGenericPlatformSentrySubsystem::TryCaptureScreenshot() const
{
	if (!isScreenshotAttachmentEnabled)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Screenshot attachment is disabled in plugin settings."));
		return;
	}

	SentryScreenshotUtils::CaptureScreenshot(GetScreenshotPath());
}

FString FGenericPlatformSentrySubsystem::GetGpuDumpBackupPath() const
{
	static const FString DateTimeString = FDateTime::Now().ToString();

	const FString GpuDumpPath = FPaths::Combine(GetDatabasePath(), TEXT("gpudumps"), *FString::Printf(TEXT("UEAftermath-%s.nv-gpudmp"), *DateTimeString));;
	const FString GpuDumpFullPath = FPaths::ConvertRelativePathToFull(GpuDumpPath);

	return GpuDumpFullPath;
}

TSharedPtr<FGenericPlatformSentryScope> FGenericPlatformSentrySubsystem::GetCurrentScope()
{
	if (scopeStack.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Scope stack is empty."));
		return nullptr;
	}

	return scopeStack.Top();
}

FString FGenericPlatformSentrySubsystem::GetHandlerPath() const
{
	const FString HandlerPath = FPaths::Combine(FSentryModule::Get().GetBinariesPath(), GetHandlerExecutableName());
	const FString HandlerFullPath = FPaths::ConvertRelativePathToFull(HandlerPath);

	return HandlerFullPath;
}

FString FGenericPlatformSentrySubsystem::GetDatabasePath() const
{
	const FString DatabasePath = FPaths::Combine(databaseParentPath, TEXT(".sentry-native"));
	const FString DatabaseFullPath = FPaths::ConvertRelativePathToFull(DatabasePath);

	return DatabaseFullPath;
}

FString FGenericPlatformSentrySubsystem::GetScreenshotPath() const
{
	const FString ScreenshotPath = FPaths::Combine(GetDatabasePath(), TEXT("screenshots"), TEXT("crash_screenshot.png"));
	const FString ScreenshotFullPath = FPaths::ConvertRelativePathToFull(ScreenshotPath);

	return ScreenshotFullPath;
}

#endif
