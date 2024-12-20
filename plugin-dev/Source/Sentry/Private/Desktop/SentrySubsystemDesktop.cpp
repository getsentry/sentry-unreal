// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemDesktop.h"
#include "SentryEventDesktop.h"
#include "SentryBreadcrumbDesktop.h"
#include "SentryUserDesktop.h"
#include "SentryUserFeedbackDesktop.h"
#include "SentryScopeDesktop.h"
#include "SentryTransactionDesktop.h"
#include "SentryTransactionContextDesktop.h"
#include "SentryIdDesktop.h"

#include "SentryDefines.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryModule.h"
#include "SentryBeforeSendHandler.h"

#include "SentryTraceSampler.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryLogUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "Infrastructure/SentryConvertorsDesktop.h"

#include "CrashReporter/SentryCrashReporter.h"
#include "CrashReporter/SentryCrashContext.h"

#include "Transport/SentryTransport.h"

#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Misc/CoreDelegates.h"
#include "HAL/FileManager.h"
#include "Misc/EngineVersionComparison.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"
#include "HAL/ExceptionHandling.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#include "Windows/WindowsPlatformCrashContext.h"
#endif

extern CORE_API bool GIsGPUCrashed;

#if USE_SENTRY_NATIVE

void PrintVerboseLog(sentry_level_t level, const char *message, va_list args, void *userdata)
{
	char buffer[512];
	vsnprintf(buffer, 512, message, args);

	FString MessageBuf = FString(buffer);

	// The WER (Windows Error Reporting) module (crashpad_wer.dll) can't be distributed along with other Sentry binaries
	// within the plugin package due to some UE Marketplace restrictions. Its absence doesn't affect crash capturing
	// and the corresponding warning can be disregarded
	if(MessageBuf.Equals(TEXT("crashpad WER handler module not found")))
	{
		return;
	}

#if !NO_LOGGING
	const FName SentryCategoryName(LogSentrySdk.GetCategoryName());
#else
	const FName SentryCategoryName(TEXT("LogSentrySdk"));
#endif

	GLog->CategorizedLogf(SentryCategoryName, SentryConvertorsDesktop::SentryLevelToLogVerbosity(level), TEXT("%s"), *MessageBuf);
}

void PrintCrashLog(const sentry_ucontext_t *uctx)
{
#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 0, 0)

	SentryConvertorsDesktop::SentryCrashContextToString(uctx, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorExceptionDescription));

	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*)GMalloc->Malloc(StackTraceSize);
	StackTrace[0] = 0;

	// Currently raw crash data stored in `uctx` can be utilized for stalk walking on Windows only
	void* ProgramCounter = uctx->exception_ptrs.ExceptionRecord->ExceptionAddress;

	FPlatformStackWalk::StackWalkAndDump(StackTrace, StackTraceSize, ProgramCounter);

	FCString::Strncat(GErrorHist, GErrorExceptionDescription, UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, TEXT("\r\n\r\n"), UE_ARRAY_COUNT(GErrorHist));
	FCString::Strncat(GErrorHist, ANSI_TO_TCHAR(StackTrace), UE_ARRAY_COUNT(GErrorHist));

#if !NO_LOGGING
	FDebug::LogFormattedMessageWithCallstack(LogSentrySdk.GetCategoryName(), __FILE__, __LINE__, TEXT("=== Critical error: ==="), GErrorHist, ELogVerbosity::Error);
#endif

#if !UE_VERSION_OLDER_THAN(5, 1, 0)
	GLog->Panic();
#endif

	GMalloc->Free(StackTrace);

#endif
}

sentry_value_t HandleBeforeSend(sentry_value_t event, void *hint, void *closure)
{
	SentrySubsystemDesktop* SentrySubsystem = static_cast<SentrySubsystemDesktop*>(closure);

	TSharedPtr<SentryEventDesktop> eventDesktop = MakeShareable(new SentryEventDesktop(event));

	SentrySubsystem->GetCurrentScope()->Apply(eventDesktop);

	FGCScopeGuard GCScopeGuard;

	USentryEvent* EventToProcess = NewObject<USentryEvent>();
	EventToProcess->InitWithNativeImpl(eventDesktop);

	USentryEvent* ProcessedEvent = EventToProcess;
	if(!FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		// Executing UFUNCTION is allowed only when not post-loading
		ProcessedEvent = SentrySubsystem->GetBeforeSendHandler()->HandleBeforeSend(EventToProcess, nullptr);
	}

	return ProcessedEvent ? event : sentry_value_new_null();
}

sentry_value_t HandleBeforeCrash(const sentry_ucontext_t *uctx, sentry_value_t event, void *closure)
{
#if PLATFORM_WINDOWS
	// Ensures that error message and corresponding callstack flushed to a log file (if available)
	// before it's attached to the captured crash event and uploaded to Sentry.
	PrintCrashLog(uctx);
#endif

	SentrySubsystemDesktop* SentrySubsystem = static_cast<SentrySubsystemDesktop*>(closure);
	SentrySubsystem->TryCaptureScreenshot();

	if (GIsGPUCrashed)
	{
		IFileManager::Get().Copy(*SentrySubsystem->GetGpuDumpBackupPath(), *SentryFileUtils::GetGpuDumpPath());
	}

	FSentryCrashContext::Get()->Apply(SentrySubsystem->GetCurrentScope());

	TSharedPtr<SentryEventDesktop> eventDesktop = MakeShareable(new SentryEventDesktop(event, true));

	SentrySubsystem->GetCurrentScope()->Apply(eventDesktop);

	if(!IsGarbageCollecting())
	{
		USentryEvent* EventToProcess = NewObject<USentryEvent>();
		EventToProcess->InitWithNativeImpl(eventDesktop);

		USentryEvent* ProcessedEvent = EventToProcess;
		if(!FUObjectThreadContext::Get().IsRoutingPostLoad)
		{
			// Executing UFUNCTION is allowed only when not post-loading
			ProcessedEvent = SentrySubsystem->GetBeforeSendHandler()->HandleBeforeSend(EventToProcess, nullptr);
		}

		return ProcessedEvent ? event : sentry_value_new_null();
	}
	else
	{
		// If crash occured during garbage collection we can't just obtain a GC lock like with normal events
		// since there is no guarantee it will be ever freed. In this case crash event will be reported
		// without calling a `beforeSend` handler.
		return event;
	}
}

SentrySubsystemDesktop::SentrySubsystemDesktop()
	: beforeSend(nullptr)
	, crashReporter(MakeShareable(new SentryCrashReporter))
	, isEnabled(false)
	, isStackTraceEnabled(true)
	, isScreenshotAttachmentEnabled(false)
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

	switch (settings->DatabaseLocation)
	{
	case ESentryDatabaseLocation::ProjectDirectory:
		databaseParentPath = FPaths::ProjectDir();
		break;
	case ESentryDatabaseLocation::ProjectUserDirectory:
		databaseParentPath = FPaths::ProjectUserDir();
		break;
	}

	if(databaseParentPath.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry database location. Falling back to FPaths::ProjectUserDir()."));
		databaseParentPath = FPaths::ProjectUserDir();
	}

	if(settings->AttachScreenshot)
	{
		isScreenshotAttachmentEnabled = true;

#if PLATFORM_WINDOWS
		sentry_options_add_attachmentw(options, *GetScreenshotPath());
#elif PLATFORM_LINUX
		sentry_options_add_attachment(options, TCHAR_TO_UTF8(*GetScreenshotPath()));
#endif
	}

	if (settings->AttachGpuDump)
	{
#if PLATFORM_WINDOWS
		sentry_options_add_attachmentw(options, *GetGpuDumpBackupPath());
#elif PLATFORM_LINUX
		sentry_options_add_attachment(options, TCHAR_TO_UTF8(*GetGpuDumpBackupPath()));
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
	sentry_options_set_handler_path(options, TCHAR_TO_UTF8(*GetHandlerPath()));
#endif

#if PLATFORM_WINDOWS
	sentry_options_set_database_pathw(options, *GetDatabasePath());
#elif PLATFORM_LINUX
	sentry_options_set_database_path(options, TCHAR_TO_UTF8(*GetDatabasePath()));
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
	sentry_options_set_shutdown_timeout(options, 3000);

#if PLATFORM_LINUX
	sentry_options_set_transport(options, FSentryTransport::Create());
#endif

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

	sentry_clear_crashed_last_run();

#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 2, 0)
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

void SentrySubsystemDesktop::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	GetCurrentScope()->AddBreadcrumb(breadcrumb);
}

void SentrySubsystemDesktop::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	TSharedPtr<SentryBreadcrumbDesktop> breadcrumbDesktop = MakeShareable(new SentryBreadcrumbDesktop());
	breadcrumbDesktop->SetMessage(Message);
	breadcrumbDesktop->SetCategory(Category);
	breadcrumbDesktop->SetType(Type);
	breadcrumbDesktop->SetData(Data);
	breadcrumbDesktop->SetLevel(Level);

	GetCurrentScope()->AddBreadcrumb(breadcrumbDesktop);
}

void SentrySubsystemDesktop::ClearBreadcrumbs()
{
	GetCurrentScope()->ClearBreadcrumbs();
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t sentryEvent = sentry_value_new_message_event(SentryConvertorsDesktop::SentryLevelToNative(level), nullptr, TCHAR_TO_UTF8(*message));

	if(isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(sentryEvent, nullptr, 0);
	}

	sentry_uuid_t id = sentry_capture_event(sentryEvent);
	return MakeShareable(new SentryIdDesktop(id));
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<SentryScopeDesktop> NewLocalScope = MakeShareable(new SentryScopeDesktop(*GetCurrentScope()));

	onScopeConfigure.ExecuteIfBound(NewLocalScope);

	scopeStack.Push(NewLocalScope);
	TSharedPtr<ISentryId> Id = CaptureMessage(message, level);
	scopeStack.Pop();

	return Id;
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<SentryEventDesktop> eventDesktop = StaticCastSharedPtr<SentryEventDesktop>(event);

	sentry_value_t nativeEvent = eventDesktop->GetNativeObject();

	if(isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

	sentry_uuid_t id = sentry_capture_event(nativeEvent);
	return MakeShareable(new SentryIdDesktop(id));
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onScopeConfigure)
{
	FScopeLock Lock(&CriticalSection);

	TSharedPtr<SentryScopeDesktop> NewLocalScope = MakeShareable(new SentryScopeDesktop(*GetCurrentScope()));

	onScopeConfigure.ExecuteIfBound(NewLocalScope);

	scopeStack.Push(NewLocalScope);
	TSharedPtr<ISentryId> Id = CaptureEvent(event);
	scopeStack.Pop();

	return Id;
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureException(const FString& type, const FString& message, int32 framesToSkip)
{
	sentry_value_t exceptionEvent = sentry_value_new_event();

	auto StackFrames = FGenericPlatformStackWalk::GetStack(framesToSkip);
	sentry_value_set_by_key(exceptionEvent, "stacktrace", SentryConvertorsDesktop::CallstackToNative(StackFrames));

	sentry_value_t nativeException = sentry_value_new_exception(TCHAR_TO_ANSI(*type), TCHAR_TO_ANSI(*message));
	sentry_event_add_exception(exceptionEvent, nativeException);

	sentry_uuid_t id = sentry_capture_event(exceptionEvent);
	return MakeShareable(new SentryIdDesktop(id));
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureAssertion(const FString& type, const FString& message)
{
#if PLATFORM_WINDOWS
	int32 framesToSkip = 7;
#else
	int32 framesToSkip = 5;
#endif

	SentryLogUtils::LogStackTrace(*message, ELogVerbosity::Error, framesToSkip);

	return CaptureException(type, message, framesToSkip);
}

TSharedPtr<ISentryId> SentrySubsystemDesktop::CaptureEnsure(const FString& type, const FString& message)
{
#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 3, 0)
	int32 framesToSkip = 8;
#else
	int32 framesToSkip = 7;
#endif
	return CaptureException(type, message, framesToSkip);
}

void SentrySubsystemDesktop::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<SentryUserFeedbackDesktop> userFeedbackDesktop = StaticCastSharedPtr<SentryUserFeedbackDesktop>(userFeedback);
	sentry_capture_user_feedback(userFeedbackDesktop->GetNativeObject());
}

void SentrySubsystemDesktop::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<SentryUserDesktop> userDesktop = StaticCastSharedPtr<SentryUserDesktop>(user);
	sentry_set_user(userDesktop->GetNativeObject());

	crashReporter->SetUser(userDesktop);
}

void SentrySubsystemDesktop::RemoveUser()
{
	sentry_remove_user();

	crashReporter->RemoveUser();
}

void SentrySubsystemDesktop::ConfigureScope(const FSentryScopeDelegate& onConfigureScope)
{
	onConfigureScope.ExecuteIfBound(GetCurrentScope());
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

TSharedPtr<ISentryTransaction> SentrySubsystemDesktop::StartTransaction(const FString& name, const FString& operation)
{
	sentry_transaction_context_t* transactionContext = sentry_transaction_context_new(TCHAR_TO_ANSI(*name), TCHAR_TO_ANSI(*operation));

	sentry_transaction_t* nativeTransaction = sentry_transaction_start(transactionContext, sentry_value_new_null());

	return MakeShareable(new SentryTransactionDesktop(nativeTransaction));
}

TSharedPtr<ISentryTransaction> SentrySubsystemDesktop::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<SentryTransactionContextDesktop> transactionContextDesktop = StaticCastSharedPtr<SentryTransactionContextDesktop>(context);

	sentry_transaction_t* nativeTransaction = sentry_transaction_start(transactionContextDesktop->GetNativeObject(), sentry_value_new_null());

	return MakeShareable(new SentryTransactionDesktop(nativeTransaction));
}

TSharedPtr<ISentryTransaction> SentrySubsystemDesktop::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	TSharedPtr<SentryTransactionContextDesktop> transactionContextDesktop = StaticCastSharedPtr<SentryTransactionContextDesktop>(context);

	sentry_transaction_t* nativeTransaction = sentry_transaction_start_ts(transactionContextDesktop->GetNativeObject(), sentry_value_new_null(), timestamp);

	return MakeShareable(new SentryTransactionDesktop(nativeTransaction));
}

TSharedPtr<ISentryTransaction> SentrySubsystemDesktop::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Transaction options currently not supported on desktop."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransactionContext> SentrySubsystemDesktop::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	sentry_transaction_context_t* nativeTransactionContext = sentry_transaction_context_new("<unlabeled transaction>", "default");
	sentry_transaction_context_update_from_header(nativeTransactionContext, "sentry-trace", TCHAR_TO_ANSI(*sentryTrace));

	// currently `sentry-native` doesn't have API for `sentry_transaction_context_t` to set `baggageHeaders`

	TSharedPtr<SentryTransactionContextDesktop> transactionContextDesktop = MakeShareable(new SentryTransactionContextDesktop(nativeTransactionContext));

	return transactionContextDesktop;
}

USentryBeforeSendHandler* SentrySubsystemDesktop::GetBeforeSendHandler()
{
	return beforeSend;
}

void SentrySubsystemDesktop::TryCaptureScreenshot() const
{
	if(!isScreenshotAttachmentEnabled)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Screenshot attachment is disabled in plugin settings."));
		return;
	}

	SentryScreenshotUtils::CaptureScreenshot(GetScreenshotPath());
}

FString SentrySubsystemDesktop::GetGpuDumpBackupPath() const
{
	static const FString DateTimeString = FDateTime::Now().ToString();

	const FString GpuDumpPath = FPaths::Combine(GetDatabasePath(), TEXT("gpudumps"), *FString::Printf(TEXT("UEAftermath-%s.nv-gpudmp"), *DateTimeString));;
	const FString GpuDumpFullPath = FPaths::ConvertRelativePathToFull(GpuDumpPath);

	return GpuDumpFullPath;
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
	const FString DatabasePath = FPaths::Combine(databaseParentPath, TEXT(".sentry-native"));
	const FString DatabaseFullPath = FPaths::ConvertRelativePathToFull(DatabasePath);

	return DatabaseFullPath;
}

FString SentrySubsystemDesktop::GetScreenshotPath() const
{
	const FString ScreenshotPath = FPaths::Combine(GetDatabasePath(), TEXT("screenshots"), TEXT("crash_screenshot.png"));
	const FString ScreenshotFullPath = FPaths::ConvertRelativePathToFull(ScreenshotPath);

	return ScreenshotFullPath;
}

#endif
