// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryEventAndroid::SentryEventAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass eventClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/SentryEvent");
	jmethodID eventCtor = Env->GetMethodID(eventClass, "<init>", "()V");
	jobject eventObject= Env->NewObject(eventClass, eventCtor);
	EventAndroid = Env->NewGlobalRef(eventObject);
}

SentryEventAndroid::SentryEventAndroid(jobject event)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	EventAndroid = Env->NewGlobalRef(event);
}

SentryEventAndroid::~SentryEventAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(EventAndroid);
}

jobject SentryEventAndroid::GetNativeObject()
{
	return EventAndroid;
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	SentryMethodCallAndroid::CallVoidMethod(EventAndroid, "setMessage", "(Lio/sentry/protocol/Message;)V",
		SentryConvertorsAndroid::SentryMessageToNative(message));
}

FString SentryEventAndroid::GetMessage() const
{
	jobject message = SentryMethodCallAndroid::CallObjectMethod(EventAndroid, "getMessage", "()Lio/sentry/protocol/Message;");
	return SentryConvertorsAndroid::SentryMessageToUnreal(message);
}

void SentryEventAndroid::SetLevel(ESentryLevel level)
{
	SentryMethodCallAndroid::CallVoidMethod(EventAndroid, "setLevel", "(Lio/sentry/SentryLevel;)V",
		SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryEventAndroid::GetLevel() const
{
	jobject level = SentryMethodCallAndroid::CallObjectMethod(EventAndroid, "getLevel", "()Lio/sentry/SentryLevel;");
	return SentryConvertorsAndroid::SentryLevelToNative(level);
}
