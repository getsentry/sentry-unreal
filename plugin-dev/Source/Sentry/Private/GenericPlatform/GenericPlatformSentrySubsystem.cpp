// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySubsystem.h"
#include "GenericPlatformSentryAttachment.h"
#include "GenericPlatformSentryBreadcrumb.h"
#include "GenericPlatformSentryEvent.h"
#include "GenericPlatformSentryId.h"
#include "GenericPlatformSentryScope.h"
#include "GenericPlatformSentryTransaction.h"
#include "GenericPlatformSentryTransactionContext.h"
#include "GenericPlatformSentryUser.h"
#include "GenericPlatformSentryUserFeedback.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "SentryTraceSampler.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryLogUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashContext.h"
#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashReporter.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/ExceptionHandling.h"
#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

extern CORE_API bool GIsGPUCrashed;

#if USE_SENTRY_NATIVE

void PrintVerboseLog(sentry_level_t level, const char* message, va_list args, void* userdata)
{
	char buffer[512];
	vsnprintf(buffer, 512, message, args);

	FString MessageBuf = FString(buffer);

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
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnBeforeSend(event, hint, closure, false);
	}
	else
	{
		return event;
	}
}

/* static */ sentry_value_t FGenericPlatformSentrySubsystem::HandleBeforeBreadcrumb(sentry_value_t breadcrumb, void* hint, void* closure)
{
	if (closure)
	{
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnBeforeBreadcrumb(breadcrumb, hint, closure);
	}
	else
	{
		return breadcrumb;
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

sentry_value_t FGenericPlatformSentrySubsystem::OnBeforeSend(sentry_value_t event, void* hint, void* closure, bool isCrash)
{
	if (!closure || this != closure)
	{
		return event;
	}

	USentryBeforeSendHandler* Handler = GetBeforeSendHandler();
	if (!Handler)
	{
		// If custom handler isn't set skip further processing
		return event;
	}

	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during object post-loading."));
		return event;
	}

	if (IsGarbageCollecting())
	{
		// If event is captured during garbage collection we can't instantiate UObjects safely or obtain a GC lock
		// since it will cause a deadlock (see https://github.com/getsentry/sentry-unreal/issues/850).
		// In this case event will be reported without calling a `beforeSend` handler.
		UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during garbage collection."));
		return event;
	}

	USentryEvent* EventToProcess = USentryEvent::Create(MakeShareable(new FGenericPlatformSentryEvent(event, isCrash)));

	USentryEvent* ProcessedEvent = Handler->HandleBeforeSend(EventToProcess, nullptr);

	return ProcessedEvent ? event : sentry_value_new_null();
}

// Currently this handler is not set anywhere since the Unreal SDK doesn't use `sentry_add_breadcrumb` directly and relies on
// custom scope implementation to store breadcrumbs instead.
// The support for it will be enabled with https://github.com/getsentry/sentry-native/pull/1166
sentry_value_t FGenericPlatformSentrySubsystem::OnBeforeBreadcrumb(sentry_value_t breadcrumb, void* hint, void* closure)
{
	if (!closure || this != closure)
	{
		return breadcrumb;
	}

	USentryBeforeBreadcrumbHandler* Handler = GetBeforeBreadcrumbHandler();
	if (!Handler)
	{
		// If custom handler isn't set skip further processing
		return breadcrumb;
	}

	if (FUObjectThreadContext::Get().IsRoutingPostLoad)
	{
		// Don't print to logs within `onBeforeBreadcrumb` handler as this can lead to creating new breadcrumb
		return breadcrumb;
	}

	if (IsGarbageCollecting())
	{
		// If breadcrumb is added during garbage collection we can't instantiate UObjects safely or obtain a GC lock
		// since there is no guarantee it will be ever freed.
		// In this case breadcrumb will be added without calling a `beforeBreadcrumb` handler.
		return breadcrumb;
	}

	USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FGenericPlatformSentryBreadcrumb(breadcrumb)));

	USentryBreadcrumb* ProcessedBreadcrumb = Handler->HandleBeforeBreadcrumb(BreadcrumbToProcess, nullptr);

	return ProcessedBreadcrumb ? breadcrumb : sentry_value_new_null();
}

sentry_value_t FGenericPlatformSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	if (isScreenshotAttachmentEnabled)
	{
		TryCaptureScreenshot();
	}

	if (GIsGPUCrashed && isGpuDumpAttachmentEnabled)
	{
		TryCaptureGpuDump();
	}

	// At this point crash events are handled the same way as non-fatal ones,
	// so we defer to `OnBeforeSend` to invoke the custom `beforeSend` handler (if configured)
	return OnBeforeSend(event, nullptr, closure, true);
}

void FGenericPlatformSentrySubsystem::InitCrashReporter(const FString& release, const FString& environment)
{
	crashReporter = MakeShareable(new FGenericPlatformSentryCrashReporter);

	crashReporter->SetRelease(release);
	crashReporter->SetEnvironment(environment);
}

void FGenericPlatformSentrySubsystem::AddFileAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	sentry_attachment_t* nativeAttachment =
		sentry_attach_file(TCHAR_TO_UTF8(*platformAttachment->GetPath()));

	if (!platformAttachment->GetFilename().IsEmpty())
		sentry_attachment_set_filename(nativeAttachment, TCHAR_TO_UTF8(*platformAttachment->GetFilename()));

	if (!platformAttachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*platformAttachment->GetContentType()));

	platformAttachment->SetNativeObject(nativeAttachment);

	attachments.Add(platformAttachment);
}

void FGenericPlatformSentrySubsystem::AddByteAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	const TArray<uint8>& byteBuf = platformAttachment->GetDataByRef();

	sentry_attachment_t* nativeAttachment =
		sentry_attach_bytes(reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), TCHAR_TO_UTF8(*platformAttachment->GetFilename()));

	if (!platformAttachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*platformAttachment->GetContentType()));

	platformAttachment->SetNativeObject(nativeAttachment);

	attachments.Add(platformAttachment);
}

FGenericPlatformSentrySubsystem::FGenericPlatformSentrySubsystem()
	: beforeSend(nullptr)
	, beforeBreadcrumb(nullptr)
	, crashReporter(nullptr)
	, isEnabled(false)
	, isStackTraceEnabled(true)
	, isPiiAttachmentEnabled(false)
	, isScreenshotAttachmentEnabled(false)
	, isGpuDumpAttachmentEnabled(false)
{
}

void FGenericPlatformSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler)
{
	beforeSend = beforeSendHandler;
	beforeBreadcrumb = beforeBreadcrumbHandler;

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

	isScreenshotAttachmentEnabled = settings->AttachScreenshot;
	if (isScreenshotAttachmentEnabled)
	{
		// Clear screenshot captured during previous session if any
		IFileManager::Get().DeleteDirectory(*FPaths::Combine(GetDatabasePath(), TEXT("screenshots")), false, true);
	}

	isGpuDumpAttachmentEnabled = settings->AttachGpuDump;

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
	ConfigureCertsPath(options);
	ConfigureNetworkConnectFunc(options);

	sentry_options_set_release(options, TCHAR_TO_ANSI(settings->OverrideReleaseName ? *settings->Release : *settings->GetFormattedReleaseName()));

	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->GetEffectiveDsn()));
	sentry_options_set_environment(options, TCHAR_TO_ANSI(*settings->Environment));
	sentry_options_set_dist(options, TCHAR_TO_ANSI(*settings->Dist));
	sentry_options_set_logger(options, PrintVerboseLog, nullptr);
	sentry_options_set_debug(options, settings->Debug);
	sentry_options_set_auto_session_tracking(options, settings->EnableAutoSessionTracking);
	sentry_options_set_sample_rate(options, settings->SampleRate);
	sentry_options_set_max_breadcrumbs(options, settings->MaxBreadcrumbs);
	sentry_options_set_before_send(options, HandleBeforeSend, this);
	sentry_options_set_on_crash(options, HandleOnCrash, this);
	sentry_options_set_shutdown_timeout(options, 3000);
	sentry_options_set_crashpad_wait_for_upload(options, settings->CrashpadWaitForUpload);

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

	sentry_clear_crashed_last_run();

	isStackTraceEnabled = settings->AttachStacktrace;
	isPiiAttachmentEnabled = settings->SendDefaultPii;
}

void FGenericPlatformSentrySubsystem::Close()
{
	isEnabled = false;

	sentry_close();
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
	if (beforeBreadcrumb != nullptr)
	{
		sentry_value_t processedBreadcrumb = HandleBeforeBreadcrumb(StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(breadcrumb)->GetNativeObject(), nullptr, this);
		if (sentry_value_is_null(processedBreadcrumb))
		{
			return;
		}
	}

	sentry_add_breadcrumb(StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(breadcrumb)->GetNativeObject());
}

void FGenericPlatformSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	TSharedPtr<FGenericPlatformSentryBreadcrumb> Breadcrumb = MakeShareable(new FGenericPlatformSentryBreadcrumb());
	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	if (beforeBreadcrumb != nullptr)
	{
		sentry_value_t processdBreadcrumb = HandleBeforeBreadcrumb(Breadcrumb->GetNativeObject(), nullptr, this);
		if (sentry_value_is_null(processdBreadcrumb))
		{
			return;
		}
	}

	sentry_add_breadcrumb(StaticCastSharedPtr<FGenericPlatformSentryBreadcrumb>(Breadcrumb)->GetNativeObject());
}

void FGenericPlatformSentrySubsystem::ClearBreadcrumbs()
{
	// Not implemented in sentry-native
}

void FGenericPlatformSentrySubsystem::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	if (!attachment->GetPath().IsEmpty())
	{
		AddFileAttachment(attachment);
	}
	else
	{
		AddByteAttachment(attachment);
	}
}

void FGenericPlatformSentrySubsystem::RemoveAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	sentry_attachment_t* nativeAttachment = platformAttachment->GetNativeObject();

	if (!nativeAttachment)
		return;

	sentry_remove_attachment(nativeAttachment);

	platformAttachment->SetNativeObject(nullptr);
}

void FGenericPlatformSentrySubsystem::ClearAttachments()
{
	for (auto& attachment : attachments)
	{
		RemoveAttachment(attachment);
	}

	attachments.Empty();
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	sentry_value_t nativeEvent = sentry_value_new_message_event(FGenericPlatformSentryConverters::SentryLevelToNative(level), nullptr, TCHAR_TO_UTF8(*message));

	if (isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

	sentry_uuid_t id = sentry_capture_event(nativeEvent);
	return MakeShareable(new FGenericPlatformSentryId(id));
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope)
{
	sentry_value_t nativeEvent = sentry_value_new_message_event(FGenericPlatformSentryConverters::SentryLevelToNative(level), nullptr, TCHAR_TO_UTF8(*message));

	if (isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

	sentry_scope_t* scope = sentry_local_scope_new();

	TSharedPtr<FGenericPlatformSentryScope> NewLocalScope = MakeShareable(new FGenericPlatformSentryScope());
	onConfigureScope.ExecuteIfBound(NewLocalScope);
	NewLocalScope->Apply(scope);

	sentry_uuid_t id = sentry_capture_event_with_scope(nativeEvent, scope);

	return MakeShareable(new FGenericPlatformSentryId(id));
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
	TSharedPtr<FGenericPlatformSentryEvent> Event = StaticCastSharedPtr<FGenericPlatformSentryEvent>(event);

	sentry_value_t nativeEvent = Event->GetNativeObject();

	if (isStackTraceEnabled)
	{
		sentry_value_set_stacktrace(nativeEvent, nullptr, 0);
	}

	sentry_scope_t* scope = sentry_local_scope_new();

	TSharedPtr<FGenericPlatformSentryScope> NewLocalScope = MakeShareable(new FGenericPlatformSentryScope());
	onScopeConfigure.ExecuteIfBound(NewLocalScope);
	NewLocalScope->Apply(scope);

	sentry_uuid_t id = sentry_capture_event_with_scope(nativeEvent, scope);

	return MakeShareable(new FGenericPlatformSentryId(id));
}

TSharedPtr<ISentryId> FGenericPlatformSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	sentry_value_t exceptionEvent = sentry_value_new_event();

	sentry_value_t nativeException = sentry_value_new_exception(TCHAR_TO_ANSI(*type), TCHAR_TO_ANSI(*message));
	sentry_event_add_exception(exceptionEvent, nativeException);

	sentry_value_set_stacktrace(exceptionEvent, nullptr, 0);

	sentry_uuid_t id = sentry_capture_event(exceptionEvent);
	return MakeShareable(new FGenericPlatformSentryId(id));
}

void FGenericPlatformSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> InUserFeedback)
{
	TSharedPtr<FGenericPlatformSentryUserFeedback> userFeedback = StaticCastSharedPtr<FGenericPlatformSentryUserFeedback>(InUserFeedback);
	sentry_capture_user_feedback(userFeedback->GetNativeObject());
}

void FGenericPlatformSentrySubsystem::SetUser(TSharedPtr<ISentryUser> InUser)
{
	TSharedPtr<FGenericPlatformSentryUser> user = StaticCastSharedPtr<FGenericPlatformSentryUser>(InUser);

	// sentry-native doesn't provide `send_default_pii` option, so we need to check if user's `ip_address`
	// allowed to be determined automatically
	if (isPiiAttachmentEnabled && user->GetIpAddress().IsEmpty())
	{
		user->SetIpAddress(TEXT("{{auto}}"));
	}

	sentry_set_user(user->GetNativeObject());

	if (crashReporter)
	{
		crashReporter->SetUser(user);
	}
}

void FGenericPlatformSentrySubsystem::RemoveUser()
{
	sentry_remove_user();

	if (crashReporter)
	{
		crashReporter->RemoveUser();
	}
}

void FGenericPlatformSentrySubsystem::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	sentry_set_context(TCHAR_TO_UTF8(*key), FGenericPlatformSentryConverters::VariantMapToNative(values));

	if (crashReporter)
	{
		crashReporter->SetContext(key, values);
	}
}

void FGenericPlatformSentrySubsystem::SetTag(const FString& key, const FString& value)
{
	sentry_set_tag(TCHAR_TO_UTF8(*key), TCHAR_TO_UTF8(*value));

	if (crashReporter)
	{
		crashReporter->SetTag(key, value);
	}
}

void FGenericPlatformSentrySubsystem::RemoveTag(const FString& key)
{
	sentry_remove_tag(TCHAR_TO_UTF8(*key));

	if (crashReporter)
	{
		crashReporter->RemoveTag(key);
	}
}

void FGenericPlatformSentrySubsystem::SetLevel(ESentryLevel level)
{
	sentry_set_level(FGenericPlatformSentryConverters::SentryLevelToNative(level));
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
	if (TSharedPtr<FGenericPlatformSentryTransactionContext> platformTransactionContext = StaticCastSharedPtr<FGenericPlatformSentryTransactionContext>(context))
	{
		if (sentry_transaction_t* nativeTransaction = sentry_transaction_start(platformTransactionContext->GetNativeObject(), sentry_value_new_null()))
		{
			return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
		}
	}

	return nullptr;
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	if (TSharedPtr<FGenericPlatformSentryTransactionContext> platformTransactionContext = StaticCastSharedPtr<FGenericPlatformSentryTransactionContext>(context))
	{
		if (sentry_transaction_t* nativeTransaction = sentry_transaction_start_ts(platformTransactionContext->GetNativeObject(), sentry_value_new_null(), timestamp))
		{
			return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
		}
	}

	return nullptr;
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

USentryBeforeBreadcrumbHandler* FGenericPlatformSentrySubsystem::GetBeforeBreadcrumbHandler()
{
	return beforeBreadcrumb;
}

void FGenericPlatformSentrySubsystem::TryCaptureScreenshot()
{
	const FString& ScreenshotPath = GetScreenshotPath();

	if (!SentryScreenshotUtils::CaptureScreenshot(ScreenshotPath))
	{
		// Screenshot capturing is a best-effort solution so if one wasn't captured skip the attachment
		return;
	}

	TSharedPtr<ISentryAttachment> ScreenshotAttachment =
		MakeShareable(new FGenericPlatformSentryAttachment(ScreenshotPath, TEXT("screenshot.png"), TEXT("image/png")));

	AddFileAttachment(ScreenshotAttachment);
}

void FGenericPlatformSentrySubsystem::TryCaptureGpuDump()
{
	const FString& GpuDumpPath = SentryFileUtils::GetGpuDumpPath();

	if (!IFileManager::Get().FileExists(*GpuDumpPath))
	{
		return;
	}

	TSharedPtr<ISentryAttachment> GpuDumpAttachment =
		MakeShareable(new FGenericPlatformSentryAttachment(GpuDumpPath, FPaths::GetCleanFilename(GpuDumpPath), TEXT("application/octet-stream")));

	AddFileAttachment(GpuDumpAttachment);
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
	const FString ScreenshotPath = FPaths::Combine(GetDatabasePath(), TEXT("screenshots"), FString::Printf(TEXT("screenshot-%s.png"), *FDateTime::Now().ToString()));
	const FString ScreenshotFullPath = FPaths::ConvertRelativePathToFull(ScreenshotPath);

	return ScreenshotFullPath;
}

#endif
