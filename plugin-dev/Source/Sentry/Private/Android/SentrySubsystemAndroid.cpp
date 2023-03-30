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

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"

const static FSentryJavaClass SentryBridgeJavaClass = FSentryJavaClass { "io/sentry/unreal/SentryBridgeJava", ESentryJavaClassType::External };
const static FSentryJavaClass SentryJavaClass = FSentryJavaClass { "io/sentry/Sentry", ESentryJavaClassType::External };

void SentrySubsystemAndroid::InitWithSettings(const USentrySettings* settings)
{
	const FString LogFilePath = settings->EnableAutoLogAttachment
		? IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename())
		: FString();

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, 
		"init", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis,
		*FJavaClassObject::GetJString(settings->DsnUrl),
		*FJavaClassObject::GetJString(settings->Release),
		*FJavaClassObject::GetJString(settings->Environment),
		*FJavaClassObject::GetJString(LogFilePath));
}

void SentrySubsystemAndroid::Close()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "close", "()V");
}

void SentrySubsystemAndroid::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbAndroid> breadcrumbAndroid = StaticCastSharedPtr<SentryBreadcrumbAndroid>(breadcrumb->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "addBreadcrumb", "(Lio/sentry/Breadcrumb;)V",
		breadcrumbAndroid->GetJObject());
}

void SentrySubsystemAndroid::ClearBreadcrumbs()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "clearBreadcrumbs", "()V");
}

USentryId* SentrySubsystemAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClass, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		*FJavaClassObject::GetJString(message), SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryBridgeJavaClass, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		*FJavaClassObject::GetJString(message), SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClass, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

USentryId* SentrySubsystemAndroid::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	auto id = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryBridgeJavaClass, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetJObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(*id);
}

void SentrySubsystemAndroid::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	TSharedPtr<SentryUserFeedbackAndroid> userFeedbackAndroid = StaticCastSharedPtr<SentryUserFeedbackAndroid>(userFeedback->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "captureUserFeedback", "(Lio/sentry/UserFeedback;)V",
		userFeedbackAndroid->GetJObject());
}

void SentrySubsystemAndroid::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserAndroid> userAndroid = StaticCastSharedPtr<SentryUserAndroid>(user->GetNativeImpl());

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "setUser", "(Lio/sentry/protocol/User;)V",
		userAndroid->GetJObject());
}

void SentrySubsystemAndroid::RemoveUser()
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClass, "setUser", "(Lio/sentry/protocol/User;)V", nullptr);
}

void SentrySubsystemAndroid::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onConfigureScope);

	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, "configureScope", "(J)V",
		(jlong)scopeCallback);
}

void SentrySubsystemAndroid::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, "setContext", "(Ljava/lang/String;Ljava/util/HashMap;)V",
		*FJavaClassObject::GetJString(key), SentryConvertorsAndroid::StringMapToNative(values)->GetJObject());
}

void SentrySubsystemAndroid::SetTag(const FString& key, const FString& value)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, "setTag", "(Ljava/lang/String;Ljava/lang/String;)V",
		*FJavaClassObject::GetJString(key), *FJavaClassObject::GetJString(value));
}

void SentrySubsystemAndroid::RemoveTag(const FString& key)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, "removeTag", "(Ljava/lang/String;)V",
		*FJavaClassObject::GetJString(key));
}

void SentrySubsystemAndroid::SetLevel(ESentryLevel level)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryBridgeJavaClass, "setLevel", "(Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::SentryLevelToNative(level)->GetJObject());
}
