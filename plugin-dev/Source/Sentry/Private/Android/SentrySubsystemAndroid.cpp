// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemAndroid.h"

#include "SentryEventAndroid.h"
#include "SentryBreadcrumbAndroid.h"
#include "SentryUserFeedbackAndroid.h"
#include "SentryUserAndroid.h"
#include "SentryDefines.h"

#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"

#include "Callbacks/SentryScopeCallbackAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"

void SentrySubsystemAndroid::InitWithSettings(const USentrySettings* settings)
{
	const FString LogFilePath = settings->EnableAutoLogAttachment
		? IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename())
		: FString();


	const FString ReleaseName = settings->OverrideReleaseName
		? settings->Release
		: settings->GetFormattedReleaseName();

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, 
		"init", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZJZ)V",
		FJavaWrapper::GameActivityThis,
		*FSentryJavaObjectWrapper::GetJString(settings->DsnUrl),
		*FSentryJavaObjectWrapper::GetJString(ReleaseName),
		*FSentryJavaObjectWrapper::GetJString(settings->Environment),
		*FSentryJavaObjectWrapper::GetJString(LogFilePath),
		settings->EnableAutoSessionTracking,
		(jlong)settings->SessionTimeout,
		settings->EnableStackTrace);
}

void SentrySubsystemAndroid::Close()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::Sentry, "close", "()V");
}

bool SentrySubsystemAndroid::IsEnabled()
{
	return FSentryJavaObjectWrapper::CallStaticMethod<bool>(SentryJavaClasses::Sentry, "isEnabled", "()Z");
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
