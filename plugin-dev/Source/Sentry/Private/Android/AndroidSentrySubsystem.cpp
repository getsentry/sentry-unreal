// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentrySubsystem.h"

#include "AndroidSentryAttachment.h"
#include "AndroidSentryBreadcrumb.h"
#include "AndroidSentryEvent.h"
#include "AndroidSentryFeedback.h"
#include "AndroidSentryId.h"
#include "AndroidSentryTransaction.h"
#include "AndroidSentryTransactionContext.h"
#include "AndroidSentryTransactionOptions.h"
#include "AndroidSentryUser.h"

#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentryTraceSampler.h"

#include "SentrySettings.h"

#include "Callbacks/AndroidSentryScopeCallback.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

#include "Utils/SentryFileUtils.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDeviceError.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Utils/SentryScreenshotUtils.h"

FAndroidSentrySubsystem::FAndroidSentrySubsystem()
{
	SentryJavaClasses::InitJavaClassRefsCache();
}

FAndroidSentrySubsystem::~FAndroidSentrySubsystem()
{
	SentryJavaClasses::ClearJavaClassRefsCache();
}

void FAndroidSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryBeforeLogHandler* beforeLogHandler, USentryTraceSampler* traceSampler)
{
	isScreenshotAttachmentEnabled = settings->AttachScreenshot;

	TSharedPtr<FJsonObject> SettingsJson = MakeShareable(new FJsonObject);
	SettingsJson->SetStringField(TEXT("dsn"), settings->Dsn);
	SettingsJson->SetStringField(TEXT("release"), settings->GetEffectiveRelease());
	SettingsJson->SetStringField(TEXT("environment"), settings->GetEffectiveEnvironment());
	SettingsJson->SetStringField(TEXT("dist"), settings->Dist);
	SettingsJson->SetBoolField(TEXT("autoSessionTracking"), settings->EnableAutoSessionTracking);
	SettingsJson->SetNumberField(TEXT("sessionTimeout"), settings->SessionTimeout);
	SettingsJson->SetBoolField(TEXT("enableStackTrace"), settings->AttachStacktrace);
	SettingsJson->SetBoolField(TEXT("debug"), settings->Debug);
	SettingsJson->SetNumberField(TEXT("sampleRate"), settings->SampleRate);
	SettingsJson->SetNumberField(TEXT("maxBreadcrumbs"), settings->MaxBreadcrumbs);
	SettingsJson->SetBoolField(TEXT("attachScreenshot"), settings->AttachScreenshot);
	SettingsJson->SetArrayField(TEXT("inAppInclude"), FAndroidSentryConverters::StrinArrayToJsonArray(settings->InAppInclude));
	SettingsJson->SetArrayField(TEXT("inAppExclude"), FAndroidSentryConverters::StrinArrayToJsonArray(settings->InAppExclude));
	SettingsJson->SetBoolField(TEXT("sendDefaultPii"), settings->SendDefaultPii);
	SettingsJson->SetBoolField(TEXT("enableAnrTracking"), settings->EnableAppNotRespondingTracking);
	SettingsJson->SetBoolField(TEXT("enableAutoLogAttachment"), settings->EnableAutoLogAttachment);
	SettingsJson->SetBoolField(TEXT("enableStructuredLogging"), settings->EnableStructuredLogging);
	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
	{
		SettingsJson->SetNumberField(TEXT("tracesSampleRate"), settings->TracesSampleRate);
	}
	if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler && traceSampler != nullptr)
	{
		SettingsJson->SetNumberField(TEXT("tracesSampler"), (jlong)traceSampler);
	}
	if (beforeBreadcrumbHandler != nullptr)
	{
		SettingsJson->SetNumberField(TEXT("beforeBreadcrumb"), (jlong)beforeBreadcrumbHandler);
	}
	if (beforeSendHandler != nullptr)
	{
		SettingsJson->SetNumberField(TEXT("beforeSendHandler"), (jlong)beforeSendHandler);
	}
	if (beforeLogHandler != nullptr)
	{
		SettingsJson->SetNumberField(TEXT("beforeLogHandler"), (jlong)beforeLogHandler);
	}

	FString SettingsJsonStr;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SettingsJsonStr);
	FJsonSerializer::Serialize(SettingsJson.ToSharedRef(), JsonWriter);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis, *FSentryJavaObjectWrapper::GetJString(SettingsJsonStr));

	if (IsEnabled() && isScreenshotAttachmentEnabled)
	{
		OnHandleSystemErrorDelegateHandle = FCoreDelegates::OnHandleSystemError.AddLambda([this]()
		{
			TryCaptureScreenshot();
		});
	}
}

void FAndroidSentrySubsystem::Close()
{
	if (OnHandleSystemErrorDelegateHandle.IsValid())
	{
		FCoreDelegates::OnHandleSystemError.Remove(OnHandleSystemErrorDelegateHandle);
		OnHandleSystemErrorDelegateHandle.Reset();
	}

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "close", "()V");
}

bool FAndroidSentrySubsystem::IsEnabled()
{
	return FSentryJavaObjectWrapper::CallStaticMethod<bool>(SentryJavaClasses::Sentry, "isEnabled", "()Z");
}

ESentryCrashedLastRun FAndroidSentrySubsystem::IsCrashedLastRun()
{
	ESentryCrashedLastRun unrealIsCrashed = ESentryCrashedLastRun::NotEvaluated;

	switch (FSentryJavaObjectWrapper::CallStaticMethod<int>(SentryJavaClasses::SentryBridgeJava, "isCrashedLastRun", "()I"))
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

void FAndroidSentrySubsystem::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<FAndroidSentryBreadcrumb> breadcrumbAndroid = StaticCastSharedPtr<FAndroidSentryBreadcrumb>(breadcrumb);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "addBreadcrumb", "(Lio/sentry/Breadcrumb;)V",
		breadcrumbAndroid->GetJObject());
}

void FAndroidSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	TSharedPtr<FAndroidSentryBreadcrumb> breadcrumbAndroid = MakeShareable(new FAndroidSentryBreadcrumb());
	breadcrumbAndroid->SetMessage(Message);
	breadcrumbAndroid->SetCategory(Category);
	breadcrumbAndroid->SetType(Type);
	breadcrumbAndroid->SetData(Data);
	breadcrumbAndroid->SetLevel(Level);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "addBreadcrumb", "(Lio/sentry/Breadcrumb;)V",
		breadcrumbAndroid->GetJObject());
}

void FAndroidSentrySubsystem::AddLog(const FString& Body, ESentryLevel Level, const FString& Category)
{
	// Ignore Empty Bodies
	if (Body.IsEmpty())
	{
		return;
	}

	// Format body with category
	FString FormattedMessage;
	if (!Category.IsEmpty())
	{
		FormattedMessage = FString::Printf(TEXT("[%s] %s"), *Category, *Body);
	}
	else
	{
		FormattedMessage = Body;
	}

	// Use level-specific Android Sentry SDK logging functions via Java bridge
	switch (Level)
	{
	case ESentryLevel::Fatal:
		FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addLogFatal", "(Ljava/lang/String;)V",
			*FSentryJavaObjectWrapper::GetJString(FormattedMessage));
		break;
	case ESentryLevel::Error:
		FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addLogError", "(Ljava/lang/String;)V",
			*FSentryJavaObjectWrapper::GetJString(FormattedMessage));
		break;
	case ESentryLevel::Warning:
		FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addLogWarn", "(Ljava/lang/String;)V",
			*FSentryJavaObjectWrapper::GetJString(FormattedMessage));
		break;
	case ESentryLevel::Info:
		FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addLogInfo", "(Ljava/lang/String;)V",
			*FSentryJavaObjectWrapper::GetJString(FormattedMessage));
		break;
	case ESentryLevel::Debug:
	default:
		FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addLogDebug", "(Ljava/lang/String;)V",
			*FSentryJavaObjectWrapper::GetJString(FormattedMessage));
		break;
	}
}

void FAndroidSentrySubsystem::ClearBreadcrumbs()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "clearBreadcrumbs", "()V");
}

void FAndroidSentrySubsystem::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAndroidSentryAttachment> attachmentAndroid = StaticCastSharedPtr<FAndroidSentryAttachment>(attachment);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "addAttachment", "(Lio/sentry/Attachment;)V",
		attachmentAndroid->GetJObject());
}

void FAndroidSentrySubsystem::RemoveAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAndroidSentryAttachment> attachmentAndroid = StaticCastSharedPtr<FAndroidSentryAttachment>(attachment);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "removeAttachment", "(Lio/sentry/Attachment;)V",
		attachmentAndroid->GetJObject());
}

void FAndroidSentrySubsystem::ClearAttachments()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "clearAttachments", "()V");
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());

	return MakeShareable(new FAndroidSentryId(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope)
{
	int64 scopeCallbackId = AndroidSentryScopeCallback::SaveDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject(), scopeCallbackId);

	return MakeShareable(new FAndroidSentryId(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<FAndroidSentryEvent> eventAndroid = StaticCastSharedPtr<FAndroidSentryEvent>(event);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject());

	return MakeShareable(new FAndroidSentryId(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope)
{
	TSharedPtr<FAndroidSentryEvent> eventAndroid = StaticCastSharedPtr<FAndroidSentryEvent>(event);

	int64 scopeCallbackId = AndroidSentryScopeCallback::SaveDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject(), scopeCallbackId);

	return MakeShareable(new FAndroidSentryId(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	TSharedPtr<FAndroidSentryAttachment> ScreenshotAttachment = nullptr;

	if (isScreenshotAttachmentEnabled)
	{
		const FString& ScreenshotPath = TryCaptureScreenshot();
		if (!ScreenshotPath.IsEmpty())
		{
			TArray<uint8> ScreenshotData;
			if (FFileHelper::LoadFileToArray(ScreenshotData, *ScreenshotPath))
			{
				ScreenshotAttachment = MakeShareable(new FAndroidSentryAttachment(ScreenshotData, TEXT("screenshot.png"), TEXT("image/png")));
			}

			if (!IFileManager::Get().Delete(*ScreenshotPath))
			{
				UE_LOG(LogSentrySdk, Error, TEXT("Failed to delete screenshot attachment: %s"), *ScreenshotPath);
			}
		}
	}

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureException", "(Ljava/lang/String;Ljava/lang/String;Lio/sentry/Attachment;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(type), *FSentryJavaObjectWrapper::GetJString(message),
		ScreenshotAttachment.IsValid() ? ScreenshotAttachment->GetJObject() : nullptr);

	return MakeShareable(new FAndroidSentryId(*id));
}

void FAndroidSentrySubsystem::CaptureFeedback(TSharedPtr<ISentryFeedback> feedback)
{
	TSharedPtr<FAndroidSentryFeedback> feedbackAndroid = StaticCastSharedPtr<FAndroidSentryFeedback>(feedback);

	FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureFeedback", "(Lio/sentry/protocol/Feedback;)Lio/sentry/protocol/SentryId;",
		feedbackAndroid->GetJObject());
}

void FAndroidSentrySubsystem::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<FAndroidSentryUser> userAndroid = StaticCastSharedPtr<FAndroidSentryUser>(user);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V",
		userAndroid->GetJObject());
}

void FAndroidSentrySubsystem::RemoveUser()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V", nullptr);
}

void FAndroidSentrySubsystem::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setContext", "(Ljava/lang/String;Ljava/util/HashMap;)V",
		*FSentryJavaObjectWrapper::GetJString(key), FAndroidSentryConverters::VariantMapToNative(values)->GetJObject());
}

void FAndroidSentrySubsystem::SetTag(const FString& key, const FString& value)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setTag", "(Ljava/lang/String;Ljava/lang/String;)V",
		*FSentryJavaObjectWrapper::GetJString(key), *FSentryJavaObjectWrapper::GetJString(value));
}

void FAndroidSentrySubsystem::RemoveTag(const FString& key)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "removeTag", "(Ljava/lang/String;)V",
		*FSentryJavaObjectWrapper::GetJString(key));
}

void FAndroidSentrySubsystem::SetLevel(ESentryLevel level)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setLevel", "(Lio/sentry/SentryLevel;)V",
		FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());
}

void FAndroidSentrySubsystem::StartSession()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "startSession", "()V", nullptr);
}

void FAndroidSentrySubsystem::EndSession()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "endSession", "()V", nullptr);
}

void FAndroidSentrySubsystem::GiveUserConsent()
{
	// No-op; feature not currently implemented for this platform
	UE_LOG(LogSentrySdk, Log, TEXT("GiveUserConsent is not supported on Android."));
}

void FAndroidSentrySubsystem::RevokeUserConsent()
{
	// No-op; feature not currently implemented for this platform
	UE_LOG(LogSentrySdk, Log, TEXT("RevokeUserConsent is not supported on Android."));
}

EUserConsent FAndroidSentrySubsystem::GetUserConsent() const
{
	UE_LOG(LogSentrySdk, Log, TEXT("GetUserConsent is not supported on Android. Returning default `Unknown` value."));
	return EUserConsent::Unknown;
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransaction(const FString& name, const FString& operation, bool bindToScope)
{
	TSharedPtr<FAndroidSentryTransactionOptions> transactionOptionsAndroid = MakeShareable(new FAndroidSentryTransactionOptions());
	transactionOptionsAndroid->SetBindToScope(bindToScope);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Ljava/lang/String;Ljava/lang/String;Lio/sentry/TransactionOptions;)Lio/sentry/ITransaction;",
		*FSentryJavaObjectWrapper::GetJString(name), *FSentryJavaObjectWrapper::GetJString(operation), transactionOptionsAndroid->GetJObject());

	return MakeShareable(new FAndroidSentryTransaction(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context, bool bindToScope)
{
	TSharedPtr<FAndroidSentryTransactionContext> transactionContextAndroid = StaticCastSharedPtr<FAndroidSentryTransactionContext>(context);

	TSharedPtr<FAndroidSentryTransactionOptions> transactionOptionsAndroid = MakeShareable(new FAndroidSentryTransactionOptions());
	transactionOptionsAndroid->SetBindToScope(bindToScope);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;Lio/sentry/TransactionOptions;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject(), transactionOptionsAndroid->GetJObject());

	return MakeShareable(new FAndroidSentryTransaction(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp, bool bindToScope)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Android."));
	return StartTransactionWithContext(context, bindToScope);
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const FSentryTransactionOptions& options)
{
	TSharedPtr<FAndroidSentryTransactionContext> transactionContextAndroid = StaticCastSharedPtr<FAndroidSentryTransactionContext>(context);

	TSharedPtr<FAndroidSentryTransactionOptions> transactionOptionsAndroid = MakeShareable(new FAndroidSentryTransactionOptions());
	transactionOptionsAndroid->SetCustomSamplingContext(options.CustomSamplingContext);
	transactionOptionsAndroid->SetBindToScope(options.BindToScope);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;Lio/sentry/TransactionOptions;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject(), transactionOptionsAndroid->GetJObject());

	return MakeShareable(new FAndroidSentryTransaction(*transaction));
}

TSharedPtr<ISentryTransactionContext> FAndroidSentrySubsystem::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	auto transactionContext = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "continueTrace", "(Ljava/lang/String;Ljava/util/List;)Lio/sentry/TransactionContext;",
		*FSentryJavaObjectWrapper::GetJString(sentryTrace), FAndroidSentryConverters::StringArrayToNative(baggageHeaders)->GetJObject());

	return MakeShareable(new FAndroidSentryTransactionContext(*transactionContext));
}

void FAndroidSentrySubsystem::HandleAssert()
{
	GError->HandleError();
	PLATFORM_BREAK();
}

FString FAndroidSentrySubsystem::TryCaptureScreenshot() const
{
	FString ScreenshotPath = SentryFileUtils::GetScreenshotPath();

	if (!SentryScreenshotUtils::CaptureScreenshot(ScreenshotPath))
	{
		return FString("");
	}

	return ScreenshotPath;
}
