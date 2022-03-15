// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAndroid.h"
#include "SentryScopeAndroid.h"
#include "SentrySettings.h"
#include "SentryMethodCallAndroid.h"
#include "SentryConvertorsAndroid.h"
#include "SentryScopeCallbackAndroid.h"

const ANSICHAR* SentryAndroid::SentryJavaClassName = "com/sentry/unreal/SentryJava";

void SentryAndroid::InitWithSettings(const USentrySettings* settings)
{
	SentryMethodCallAndroid::CallStaticVoidMethod(SentryJavaClassName, "init", "(Landroid/app/Activity;Ljava/lang/String;)V",
		FJavaWrapper::GameActivityThis,
		SentryConvertorsAndroid::StringToNative(settings->DsnUrl));
}

FString SentryAndroid::CaptureMessage(const FString& message, ESentryLevel level)
{
	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureMessage", "(Ljava/lang/String;I)Ljava/lang/String;",
		SentryConvertorsAndroid::StringToNative(message), (int)level);
}

FString SentryAndroid::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	USentryScopeCallbackAndroid* scopeCallback = NewObject<USentryScopeCallbackAndroid>();
	scopeCallback->BindDelegate(onScopeConfigure);
	return SentryMethodCallAndroid::CallStaticStringMethod(SentryJavaClassName, "captureMessageWithScope", "(Ljava/lang/String;IJ)Ljava/lang/String;",
		SentryConvertorsAndroid::StringToNative(message), (int)level, (jlong)scopeCallback);
}

JNI_METHOD void Java_com_sentry_unreal_SentryJava_onConfigureScope(JNIEnv* env, jclass clazz, jlong objAddr, jobject scope)
{
	USentryScopeCallbackAndroid* callback = reinterpret_cast<USentryScopeCallbackAndroid*>(objAddr);

	if (IsValid(callback))
	{
		TSharedPtr<SentryScopeAndroid> scopeNativeImpl = MakeShareable(new SentryScopeAndroid());
		scopeNativeImpl->InitWithNativeObject(scope);

		callback->ExecuteDelegate(SentryConvertorsAndroid::SentryScopeToUnreal(scopeNativeImpl));
	}
}
