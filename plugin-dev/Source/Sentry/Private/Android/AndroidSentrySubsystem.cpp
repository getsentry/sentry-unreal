// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentrySubsystem.h"

#include "AndroidSentryAttachment.h"
#include "AndroidSentryBreadcrumb.h"
#include "AndroidSentryEvent.h"
#include "AndroidSentryId.h"
#include "AndroidSentryTransaction.h"
#include "AndroidSentryTransactionContext.h"
#include "AndroidSentryTransactionOptions.h"
#include "AndroidSentryUser.h"
#include "AndroidSentryUserFeedback.h"

#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentryTraceSampler.h"

#include "SentrySettings.h"

#include "Callbacks/AndroidSentryScopeCallback.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

#include "Utils/SentryFileUtils.h"

#include "Dom/JsonObject.h"
#include "Misc/OutputDeviceError.h"
#include "Serialization/JsonSerializer.h"

void FAndroidSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler)
{
	TSharedPtr<FJsonObject> SettingsJson = MakeShareable(new FJsonObject);
	SettingsJson->SetStringField(TEXT("dsn"), settings->Dsn);
	SettingsJson->SetStringField(TEXT("release"), settings->OverrideReleaseName ? settings->Release : settings->GetFormattedReleaseName());
	SettingsJson->SetStringField(TEXT("environment"), settings->Environment);
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

	FString SettingsJsonStr;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SettingsJsonStr);
	FJsonSerializer::Serialize(SettingsJson.ToSharedRef(), JsonWriter);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis, *FSentryJavaObjectWrapper::GetJString(SettingsJsonStr));
}

void FAndroidSentrySubsystem::Close()
{
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
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureException", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(type), *FSentryJavaObjectWrapper::GetJString(message));

	return MakeShareable(new FAndroidSentryId(*id));
}

void FAndroidSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<FAndroidSentryUserFeedback> userFeedbackAndroid = StaticCastSharedPtr<FAndroidSentryUserFeedback>(userFeedback);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "captureUserFeedback", "(Lio/sentry/UserFeedback;)V",
		userFeedbackAndroid->GetJObject());
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

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransaction(const FString& name, const FString& operation)
{
	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ITransaction;",
		*FSentryJavaObjectWrapper::GetJString(name), *FSentryJavaObjectWrapper::GetJString(operation));

	return MakeShareable(new FAndroidSentryTransaction(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<FAndroidSentryTransactionContext> transactionContextAndroid = StaticCastSharedPtr<FAndroidSentryTransactionContext>(context);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject());

	return MakeShareable(new FAndroidSentryTransaction(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Android."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	TSharedPtr<FAndroidSentryTransactionContext> transactionContextAndroid = StaticCastSharedPtr<FAndroidSentryTransactionContext>(context);

	TSharedPtr<FAndroidSentryTransactionOptions> transactionOptionsAndroid = MakeShareable(new FAndroidSentryTransactionOptions());
	transactionOptionsAndroid->SetCustomSamplingContext(options);

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
