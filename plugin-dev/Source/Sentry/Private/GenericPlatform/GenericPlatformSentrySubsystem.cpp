// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentrySubsystem.h"
#include "GenericPlatformSentryAttachment.h"
#include "GenericPlatformSentryBreadcrumb.h"
#include "GenericPlatformSentryEvent.h"
#include "GenericPlatformSentryFeedback.h"
#include "GenericPlatformSentryId.h"
#include "GenericPlatformSentryLog.h"
#include "GenericPlatformSentrySamplingContext.h"
#include "GenericPlatformSentryScope.h"
#include "GenericPlatformSentryTransaction.h"
#include "GenericPlatformSentryTransactionContext.h"
#include "GenericPlatformSentryUser.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeLogHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryLog.h"
#include "SentryModule.h"
#include "SentrySamplingContext.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryTraceSampler.h"

#include "Utils/SentryCallbackUtils.h"
#include "Utils/SentryFileUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashContext.h"
#include "GenericPlatform/CrashReporter/GenericPlatformSentryCrashReporter.h"

#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/ExceptionHandling.h"
#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"
#include "UObject/UObjectThreadContext.h"

extern CORE_API bool GIsGPUCrashed;

#if USE_SENTRY_NATIVE

static void PrintVerboseLog(sentry_level_t level, const char* message, va_list args, void* closure)
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

/* static */ double FGenericPlatformSentrySubsystem::HandleTraceSampling(const sentry_transaction_context_t* transaction_ctx, sentry_value_t custom_sampling_ctx, const int* parent_sampled, void* closure)
{
	if (closure)
	{
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnTraceSampling(transaction_ctx, custom_sampling_ctx, parent_sampled);
	}
	else
	{
		return parent_sampled != nullptr ? *parent_sampled : 0.0;
	}
}

/* static */ sentry_value_t FGenericPlatformSentrySubsystem::HandleBeforeLog(sentry_value_t log, void* closure)
{
	if (closure)
	{
		return StaticCast<FGenericPlatformSentrySubsystem*>(closure)->OnBeforeLog(log, closure);
	}

	return log;
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

	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
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

	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		return breadcrumb;
	}

	USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FGenericPlatformSentryBreadcrumb(breadcrumb)));

	USentryBreadcrumb* ProcessedBreadcrumb = Handler->HandleBeforeBreadcrumb(BreadcrumbToProcess, nullptr);

	return ProcessedBreadcrumb ? breadcrumb : sentry_value_new_null();
}

sentry_value_t FGenericPlatformSentrySubsystem::OnBeforeLog(sentry_value_t log, void* closure)
{
	if (!closure || this != closure)
	{
		return log;
	}

	USentryBeforeLogHandler* Handler = GetBeforeLogHandler();
	if (!Handler)
	{
		// If custom handler isn't set skip further processing
		return log;
	}

	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		return log;
	}

	// Create USentryLog object using the log wrapper
	USentryLog* LogData = USentryLog::Create(MakeShareable(new FGenericPlatformSentryLog(log)));

	USentryLog* ProcessedLogData = Handler->HandleBeforeLog(LogData);

	return ProcessedLogData ? log : sentry_value_new_null();
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

double FGenericPlatformSentrySubsystem::OnTraceSampling(const sentry_transaction_context_t* transaction_ctx, sentry_value_t custom_sampling_ctx, const int* parent_sampled)
{
	USentryTraceSampler* Sampler = GetTraceSampler();
	if (!Sampler)
	{
		// If custom sampler isn't set skip further processing
		return parent_sampled != nullptr ? *parent_sampled : 0.0;
	}

	if (!SentryCallbackUtils::IsCallbackSafeToRun())
	{
		return parent_sampled != nullptr ? *parent_sampled : 0.0;
	}

	USentrySamplingContext* Context = USentrySamplingContext::Create(
		MakeShareable(new FGenericPlatformSentrySamplingContext(const_cast<sentry_transaction_context_t*>(transaction_ctx), custom_sampling_ctx)));

	float samplingValue;
	if (Sampler->Sample(Context, samplingValue))
	{
		return samplingValue;
	}

	return parent_sampled != nullptr ? *parent_sampled : 0.0;
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
	, beforeLog(nullptr)
	, sampler(nullptr)
	, crashReporter(nullptr)
	, isEnabled(false)
	, isStackTraceEnabled(true)
	, isPiiAttachmentEnabled(false)
	, isScreenshotAttachmentEnabled(false)
	, isGpuDumpAttachmentEnabled(false)
{
}

void FGenericPlatformSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryBeforeLogHandler* beforeLogHandler, USentryTraceSampler* traceSampler)
{
	beforeSend = beforeSendHandler;
	beforeBreadcrumb = beforeBreadcrumbHandler;
	beforeLog = beforeLogHandler;
	sampler = traceSampler;

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
		sentry_options_set_proxy(options, TCHAR_TO_ANSI(*settings->ProxyUrl));
	}

	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
	{
		sentry_options_set_traces_sample_rate(options, settings->TracesSampleRate);
	}
	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
	{
		sentry_options_set_traces_sampler(options, HandleTraceSampling, this);
	}

	ConfigureHandlerPath(options);
	ConfigureDatabasePath(options);
	ConfigureCertsPath(options);
	ConfigureNetworkConnectFunc(options);

	sentry_options_set_dsn(options, TCHAR_TO_ANSI(*settings->GetEffectiveDsn()));
	sentry_options_set_release(options, TCHAR_TO_ANSI(*settings->GetEffectiveRelease()));
	sentry_options_set_environment(options, TCHAR_TO_ANSI(*settings->GetEffectiveEnvironment()));
	sentry_options_set_dist(options, TCHAR_TO_ANSI(*settings->Dist));
	sentry_options_set_logger(options, PrintVerboseLog, nullptr);
	sentry_options_set_debug(options, settings->Debug);
	sentry_options_set_auto_session_tracking(options, settings->EnableAutoSessionTracking);
	sentry_options_set_sample_rate(options, settings->SampleRate);
	sentry_options_set_max_breadcrumbs(options, settings->MaxBreadcrumbs);
	sentry_options_set_before_send(options, HandleBeforeSend, this);
	sentry_options_set_before_send_log(options, HandleBeforeLog, this);
	sentry_options_set_on_crash(options, HandleOnCrash, this);
	sentry_options_set_shutdown_timeout(options, 3000);
	sentry_options_set_crashpad_wait_for_upload(options, settings->CrashpadWaitForUpload);
	sentry_options_set_logger_enabled_when_crashed(options, false);
	sentry_options_set_enable_logs(options, settings->EnableStructuredLogging);

	if (settings->bRequireUserConsent)
	{
		sentry_options_set_require_user_consent(options, 1);
	}

	int initResult = sentry_init(options);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization completed with result %d (0 on success)."), initResult);

	isEnabled = initResult == 0 ? true : false;

	sentry_clear_crashed_last_run();

	isStackTraceEnabled = settings->AttachStacktrace;
	isPiiAttachmentEnabled = settings->SendDefaultPii;

	// Best-effort at writing user consent to disk so that user consent can change at runtime and persist
	// We should never have a valid user consent state return "Unknown", so assume that no consent value is written if we see this
	if (settings->bRequireUserConsent && GetUserConsent() == EUserConsent::Unknown)
	{
		if (settings->bDefaultUserConsentGiven)
		{
			GiveUserConsent();
		}
		else
		{
			RevokeUserConsent();
		}
	}
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

void FGenericPlatformSentrySubsystem::AddLog(const FString& Body, ESentryLevel Level, const FString& Category)
{
	// Ignore Empty Bodies
	if (Body.IsEmpty())
	{
		return;
	}

	// Format body with category if provided
	FString FormattedMessage;
	if (!Category.IsEmpty())
	{
		FormattedMessage = FString::Printf(TEXT("[%s] %s"), *Category, *Body);
	}
	else
	{
		FormattedMessage = Body;
	}

	auto MessageCStrConverter = StringCast<ANSICHAR>(*FormattedMessage);
	const char* MessageCStr = MessageCStrConverter.Get();

	// Use level-specific sentry logging functions
	switch (Level)
	{
	case ESentryLevel::Fatal:
		sentry_log_fatal(MessageCStr);
		break;
	case ESentryLevel::Error:
		sentry_log_error(MessageCStr);
		break;
	case ESentryLevel::Warning:
		sentry_log_warn(MessageCStr);
		break;
	case ESentryLevel::Info:
		sentry_log_info(MessageCStr);
		break;
	case ESentryLevel::Debug:
	default:
		sentry_log_debug(MessageCStr);
		break;
	}
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

void FGenericPlatformSentrySubsystem::CaptureFeedback(TSharedPtr<ISentryFeedback> feedback)
{
	TSharedPtr<FGenericPlatformSentryFeedback> Feedback = StaticCastSharedPtr<FGenericPlatformSentryFeedback>(feedback);
	sentry_capture_feedback(Feedback->GetNativeObject());
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

void FGenericPlatformSentrySubsystem::GiveUserConsent()
{
	sentry_user_consent_give();
}

void FGenericPlatformSentrySubsystem::RevokeUserConsent()
{
	sentry_user_consent_revoke();
}

EUserConsent FGenericPlatformSentrySubsystem::GetUserConsent() const
{
	switch (sentry_user_consent_get())
	{
	case 0:
		return EUserConsent::Revoked;
	case 1:
		return EUserConsent::Given;
	default:
		return EUserConsent::Unknown;
	}
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransaction(const FString& name, const FString& operation, bool bindToScope)
{
	TSharedPtr<ISentryTransactionContext> transactionContext = MakeShareable(new FGenericPlatformSentryTransactionContext(name, operation));

	return StartTransactionWithContext(transactionContext, bindToScope);
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context, bool bindToScope)
{
	if (TSharedPtr<FGenericPlatformSentryTransactionContext> platformTransactionContext = StaticCastSharedPtr<FGenericPlatformSentryTransactionContext>(context))
	{
		if (sentry_transaction_t* nativeTransaction = sentry_transaction_start(platformTransactionContext->GetNativeObject(), sentry_value_new_null()))
		{
			if (bindToScope)
			{
				sentry_set_transaction_object(nativeTransaction);
			}

			return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
		}
	}

	return nullptr;
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp, bool bindToScope)
{
	if (TSharedPtr<FGenericPlatformSentryTransactionContext> platformTransactionContext = StaticCastSharedPtr<FGenericPlatformSentryTransactionContext>(context))
	{
		if (sentry_transaction_t* nativeTransaction = sentry_transaction_start_ts(platformTransactionContext->GetNativeObject(), sentry_value_new_null(), timestamp))
		{
			if (bindToScope)
			{
				sentry_set_transaction_object(nativeTransaction);
			}

			return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
		}
	}

	return nullptr;
}

TSharedPtr<ISentryTransaction> FGenericPlatformSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const FSentryTransactionOptions& options)
{
	if (TSharedPtr<FGenericPlatformSentryTransactionContext> platformTransactionContext = StaticCastSharedPtr<FGenericPlatformSentryTransactionContext>(context))
	{
		if (sentry_transaction_t* nativeTransaction = sentry_transaction_start(platformTransactionContext->GetNativeObject(), FGenericPlatformSentryConverters::VariantMapToNative(options.CustomSamplingContext)))
		{
			if (options.BindToScope)
			{
				sentry_set_transaction_object(nativeTransaction);
			}

			return MakeShareable(new FGenericPlatformSentryTransaction(nativeTransaction));
		}
	}

	return nullptr;
}

TSharedPtr<ISentryTransactionContext> FGenericPlatformSentrySubsystem::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	TSharedPtr<FGenericPlatformSentryTransactionContext> transactionContext = MakeShareable(new FGenericPlatformSentryTransactionContext(TEXT("<unlabeled transaction>"), TEXT("default")));

	sentry_transaction_context_update_from_header(transactionContext->GetNativeObject(), "sentry-trace", TCHAR_TO_ANSI(*sentryTrace));

	// currently `sentry-native` doesn't have API for `sentry_transaction_context_t` to set `baggageHeaders`

	return transactionContext;
}

USentryBeforeSendHandler* FGenericPlatformSentrySubsystem::GetBeforeSendHandler() const
{
	return beforeSend;
}

USentryBeforeBreadcrumbHandler* FGenericPlatformSentrySubsystem::GetBeforeBreadcrumbHandler() const
{
	return beforeBreadcrumb;
}

USentryBeforeLogHandler* FGenericPlatformSentrySubsystem::GetBeforeLogHandler() const
{
	return beforeLog;
}

USentryTraceSampler* FGenericPlatformSentrySubsystem::GetTraceSampler() const
{
	return sampler;
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
