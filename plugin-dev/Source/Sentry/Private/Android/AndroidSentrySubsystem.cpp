// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "AndroidSentrySubsystem.h"

#include "SentryEventAndroid.h"
#include "SentryBreadcrumbAndroid.h"
#include "SentryUserFeedbackAndroid.h"
#include "SentryUserAndroid.h"
#include "SentryTransactionAndroid.h"
#include "SentryTransactionContextAndroid.h"
#include "SentryTransactionOptionsAndroid.h"
#include "SentryIdAndroid.h"

#include "SentryDefines.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentryEvent.h"

#include "SentrySettings.h"

#include "Callbacks/SentryScopeCallbackAndroid.h"

#include "Infrastructure/SentryConvertersAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

#include "Utils/SentryFileUtils.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

void FAndroidSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	TSharedPtr<FJsonObject> SettingsJson = MakeShareable(new FJsonObject);
	SettingsJson->SetStringField(TEXT("dsn"), settings->Dsn);
	SettingsJson->SetStringField(TEXT("release"), settings->OverrideReleaseName
		? settings->Release
		: settings->GetFormattedReleaseName());
	SettingsJson->SetStringField(TEXT("environment"), settings->Environment);
	SettingsJson->SetBoolField(TEXT("autoSessionTracking"), settings->EnableAutoSessionTracking);
	SettingsJson->SetNumberField(TEXT("sessionTimeout"), settings->SessionTimeout);
	SettingsJson->SetBoolField(TEXT("enableStackTrace"), settings->AttachStacktrace);
	SettingsJson->SetBoolField(TEXT("debug"), settings->Debug);
	SettingsJson->SetNumberField(TEXT("sampleRate"), settings->SampleRate);
	SettingsJson->SetNumberField(TEXT("maxBreadcrumbs"), settings->MaxBreadcrumbs);
	SettingsJson->SetBoolField(TEXT("attachScreenshot"), settings->AttachScreenshot);
	SettingsJson->SetArrayField(TEXT("inAppInclude"), SentryConvertersAndroid::StrinArrayToJsonArray(settings->InAppInclude));
	SettingsJson->SetArrayField(TEXT("inAppExclude"), SentryConvertersAndroid::StrinArrayToJsonArray(settings->InAppExclude));
	SettingsJson->SetBoolField(TEXT("sendDefaultPii"), settings->SendDefaultPii);
	SettingsJson->SetBoolField(TEXT("enableAnrTracking"), settings->EnableAppNotRespondingTracking);
	if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
	{
		SettingsJson->SetNumberField(TEXT("tracesSampleRate"), settings->TracesSampleRate);
	}
	if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
	{
		SettingsJson->SetNumberField(TEXT("tracesSampler"), (jlong)traceSampler);
	}

	FString SettingsJsonStr;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&SettingsJsonStr);
	FJsonSerializer::Serialize(SettingsJson.ToSharedRef(), JsonWriter);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, 
		"init", "(Landroid/app/Activity;Ljava/lang/String;J)V",
		FJavaWrapper::GameActivityThis,
		*FSentryJavaObjectWrapper::GetJString(SettingsJsonStr),
		(jlong)beforeSendHandler);
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
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = StaticCastSharedPtr<SentryBreadcrumbAndroid>(breadcrumb);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "addBreadcrumb", "(Lio/sentry/Breadcrumb;)V",
		breadcrumbAndroid->GetJObject());
}

void FAndroidSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = MakeShareable(new SentryBreadcrumbAndroid());
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

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), SentryConvertersAndroid::SentryLevelToNative(level)->GetJObject());

	return MakeShareable(new SentryIdAndroid(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onConfigureScope, ESentryLevel level)
{
	int64 scopeCallbackId = SentryScopeCallbackAndroid::SaveDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), SentryConvertersAndroid::SentryLevelToNative(level)->GetJObject(), scopeCallbackId);

	return MakeShareable(new SentryIdAndroid(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject());

	return MakeShareable(new SentryIdAndroid(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event);

	int64 scopeCallbackId = SentryScopeCallbackAndroid::SaveDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject(), scopeCallbackId);

	return MakeShareable(new SentryIdAndroid(*id));
}

TSharedPtr<ISentryId> FAndroidSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureException", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(type), *FSentryJavaObjectWrapper::GetJString(message));

	return MakeShareable(new SentryIdAndroid(*id));
}

void FAndroidSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<SentryUserFeedbackAndroid> userFeedbackAndroid = StaticCastSharedPtr<SentryUserFeedbackAndroid>(userFeedback);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "captureUserFeedback", "(Lio/sentry/UserFeedback;)V",
		userFeedbackAndroid->GetJObject());
}

void FAndroidSentrySubsystem::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<SentryUserAndroid> userAndroid = StaticCastSharedPtr<SentryUserAndroid>(user);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V",
		userAndroid->GetJObject());
}

void FAndroidSentrySubsystem::RemoveUser()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V", nullptr);
}

void FAndroidSentrySubsystem::ConfigureScope(const FSentryScopeDelegate& onConfigureScope)
{
	int64 scopeCallbackId = SentryScopeCallbackAndroid::SaveDelegate(onConfigureScope);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "configureScope", "(J)V", scopeCallbackId);
}

void FAndroidSentrySubsystem::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setContext", "(Ljava/lang/String;Ljava/util/HashMap;)V",
		*FSentryJavaObjectWrapper::GetJString(key), SentryConvertersAndroid::StringMapToNative(values)->GetJObject());
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
		SentryConvertersAndroid::SentryLevelToNative(level)->GetJObject());
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

	return MakeShareable(new SentryTransactionAndroid(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<SentryTransactionContextAndroid> transactionContextAndroid = StaticCastSharedPtr<SentryTransactionContextAndroid>(context);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject());

	return MakeShareable(new SentryTransactionAndroid(*transaction));
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Android."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransaction> FAndroidSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	TSharedPtr<SentryTransactionContextAndroid> transactionContextAndroid = StaticCastSharedPtr<SentryTransactionContextAndroid>(context);

	TSharedPtr<SentryTransactionOptionsAndroid> transactionOptionsAndroid = MakeShareable(new SentryTransactionOptionsAndroid());
	transactionOptionsAndroid->SetCustomSamplingContext(options);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;Lio/sentry/TransactionOptions;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject(), transactionOptionsAndroid->GetJObject());

	return MakeShareable(new SentryTransactionAndroid(*transaction));
}

TSharedPtr<ISentryTransactionContext> FAndroidSentrySubsystem::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	auto transactionContext = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "continueTrace", "(Ljava/lang/String;Ljava/util/List;)Lio/sentry/TransactionContext;",
		*FSentryJavaObjectWrapper::GetJString(sentryTrace), SentryConvertersAndroid::StringArrayToNative(baggageHeaders)->GetJObject());

	return MakeShareable(new SentryTransactionContextAndroid(*transactionContext));
}
