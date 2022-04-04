// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemAndroid.h"
#include "SentryScopeAndroid.h"
#include "SentryEventAndroid.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "Callbacks/SentryScopeCallbackAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

const ANSICHAR* SentrySubsystemAndroid::SentryJavaClassName = "com/sentry/unreal/SentryJava";

void SentrySubsystemAndroid::InitWithSettings(const USentrySettings* settings)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis,
		SentryConvertorsAndroid::StringToNative(settings->DsnUrl));
}

void SentrySubsystemAndroid::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "addBreadcrumb", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::StringToNative(message),
		SentryConvertorsAndroid::StringToNative(category),
		SentryConvertorsAndroid::StringToNative(type),
		SentryConvertorsAndroid::StringMapToNative(data),
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

USentryId* SentrySubsystemAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Lio/sentry/protocol/SentryId;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level));

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onScopeConfigure);

	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Lio/sentry/protocol/SentryId;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level), (jlong)scopeCallback);
		
	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	jobject id =  SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureEvent", "(Lio/sentry/SentryEvent;)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetNativeObject());

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemAndroid::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onScopeConfigure);

	jobject id = SentryMethodCallAndroid::CallStaticObjectMethod(SentryJavaClassName, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Lio/sentry/protocol/SentryId;",
		eventAndroid->GetNativeObject(), (jlong)scopeCallback);

	return SentryConvertorsAndroid::SentryIdToUnreal(id);
}
