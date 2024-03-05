// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemAndroid.h"

#include "SentryEventAndroid.h"
#include "SentryBreadcrumbAndroid.h"
#include "SentryUserFeedbackAndroid.h"
#include "SentryUserAndroid.h"
#include "SentryTransactionContextAndroid.h"
#include "SentryTransactionOptionsAndroid.h"

#include "SentryDefines.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"
#include "SentryTransactionContext.h"

#include "Callbacks/SentryScopeCallbackAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

#include "Utils/SentryFileUtils.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

void SentrySubsystemAndroid::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
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
	SettingsJson->SetArrayField(TEXT("inAppInclude"), SentryConvertorsAndroid::StrinArrayToJsonArray(settings->InAppInclude));
	SettingsJson->SetArrayField(TEXT("inAppExclude"), SentryConvertorsAndroid::StrinArrayToJsonArray(settings->InAppExclude));
	SettingsJson->SetBoolField(TEXT("sendDefaultPii"), settings->SendDefaultPii);
	SettingsJson->SetBoolField(TEXT("enableTracing"), settings->EnableTracing);
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

void SentrySubsystemAndroid::Close()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "close", "()V");
}

bool SentrySubsystemAndroid::IsEnabled()
{
	return FSentryJavaObjectWrapper::CallStaticMethod<bool>(SentryJavaClasses::Sentry, "isEnabled", "()Z");
}

ESentryCrashedLastRun SentrySubsystemAndroid::IsCrashedLastRun()
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

void SentrySubsystemAndroid::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = StaticCastSharedPtr<SentryBreadcrumbAndroid>(breadcrumb->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "addBreadcrumb", "(Lio/sentry/Breadcrumb;)V",
		breadcrumbAndroid->GetJObject());
}

void SentrySubsystemAndroid::ClearBreadcrumbs()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "clearBreadcrumbs", "()V");
}

USentryId* SentrySubsystemAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		*FSentryJavaObjectWrapper::GetJString(message), SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

void SentrySubsystemAndroid::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	TSharedPtr<SentryUserFeedbackAndroid> userFeedbackAndroid = StaticCastSharedPtr<SentryUserFeedbackAndroid>(userFeedback->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "captureUserFeedback", "(Lio/sentry/UserFeedback;)V",
		userFeedbackAndroid->GetJObject());
}

void SentrySubsystemAndroid::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserAndroid> userAndroid = StaticCastSharedPtr<SentryUserAndroid>(user->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V",
		userAndroid->GetJObject());
}

void SentrySubsystemAndroid::RemoveUser()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "setUser", "(Lio/sentry/protocol/User;)V", nullptr);
}

void SentrySubsystemAndroid::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "configureScope", "(J)V",
		(jlong)scopeCallback);
}

void SentrySubsystemAndroid::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setContext", "(Ljava/lang/String;Ljava/util/HashMap;)V",
		*FSentryJavaObjectWrapper::GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentrySubsystemAndroid::SetTag(const FString& key, const FString& value)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setTag", "(Ljava/lang/String;Ljava/lang/String;)V",
		*FSentryJavaObjectWrapper::GetJString(key), *FSentryJavaObjectWrapper::GetJString(value));
}

void SentrySubsystemAndroid::RemoveTag(const FString& key)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "removeTag", "(Ljava/lang/String;)V",
		*FSentryJavaObjectWrapper::GetJString(key));
}

void SentrySubsystemAndroid::SetLevel(ESentryLevel level)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setLevel", "(Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}

void SentrySubsystemAndroid::StartSession()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "startSession", "()V", nullptr);
}

void SentrySubsystemAndroid::EndSession()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "endSession", "()V", nullptr);
}

USentryTransaction* SentrySubsystemAndroid::StartTransaction(const FString& name, const FString& operation)
{
	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Ljava/lang/String;Ljava/lang/String;)Lio/sentry/ITransaction;",
		*FSentryJavaObjectWrapper::GetJString(name), *FSentryJavaObjectWrapper::GetJString(operation));

	return SentryConvertorsAndroid::SentryTransactionToUnreal(*transaction);
}

USentryTransaction* SentrySubsystemAndroid::StartTransactionWithContext(USentryTransactionContext* context)
{
	TSharedPtr<SentryTransactionContextAndroid> transactionContextAndroid = StaticCastSharedPtr<SentryTransactionContextAndroid>(context->GetNativeImpl());

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject());

	return SentryConvertorsAndroid::SentryTransactionToUnreal(*transaction);
}

USentryTransaction* SentrySubsystemAndroid::StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options)
{
	TSharedPtr<SentryTransactionContextAndroid> transactionContextAndroid = StaticCastSharedPtr<SentryTransactionContextAndroid>(context->GetNativeImpl());

	TSharedPtr<SentryTransactionOptionsAndroid> transactionOptionsAndroid = MakeShareable(new SentryTransactionOptionsAndroid());
	transactionOptionsAndroid->SetCustomSamplingContext(options);

	auto transaction = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::Sentry, "startTransaction", "(Lio/sentry/TransactionContext;Lio/sentry/TransactionOptions;)Lio/sentry/ITransaction;",
		transactionContextAndroid->GetJObject(), transactionOptionsAndroid->GetJObject());

	return SentryConvertorsAndroid::SentryTransactionToUnreal(*transaction);
}
