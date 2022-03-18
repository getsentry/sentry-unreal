// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAndroid.h"
#include "SentryScopeAndroid.h"
#include "SentryEventAndroid.h"
#include "SentrySettings.h"
#include "SentryEvent.h"
#include "Callbacks/SentryScopeCallbackAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

const ANSICHAR* SentryAndroid::SentryJavaClassName = "com/sentry/unreal/SentryJava";

void SentryAndroid::InitWithSettings(const USentrySettings* settings)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis,
		SentryConvertorsAndroid::StringToNative(settings->DsnUrl));
}

void SentryAndroid::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "addBreadcrumb", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/HashMap;Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::StringToNative(message),
		SentryConvertorsAndroid::StringToNative(category),
		SentryConvertorsAndroid::StringToNative(type),
		SentryConvertorsAndroid::StringMapToNative(data),
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

FString SentryAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureMessage", "(Ljava/lang/String;Lio/sentry/SentryLevel;)Ljava/lang/String;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level));
}

FString SentryAndroid::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onScopeConfigure);

	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureMessageWithScope", "(Ljava/lang/String;Lio/sentry/SentryLevel;J)Ljava/lang/String;",
		SentryConvertorsAndroid::StringToNative(message), SentryConvertorsAndroid::SentryLevelToNative(level), (jlong)scopeCallback);
}

FString SentryAndroid::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureEvent", "(Lio/sentry/SentryEvent;)Ljava/lang/String;",
		eventAndroid->GetNativeObject());
}

FString SentryAndroid::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	TSharedPtr<SentryEventAndroid> eventAndroid = StaticCastSharedPtr<SentryEventAndroid>(event->GetNativeImpl());

	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onScopeConfigure);

	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureEventWithScope", "(Lio/sentry/SentryEvent;J)Ljava/lang/String;",
		eventAndroid->GetNativeObject(), (jlong)scopeCallback);
}
