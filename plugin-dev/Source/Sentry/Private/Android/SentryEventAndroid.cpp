// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

#include "Android/AndroidApplication.h"

SentryEventAndroid::SentryEventAndroid()
	: FJavaClassObject(GetClassName(), "()V")
	, SetMessageMethod(GetClassMethod("setMessage", "(Lio/sentry/protocol/Message;)V"))
	, GetMessageMethod(GetClassMethod("getMessage", "()Lio/sentry/protocol/Message;"))
	, SetLevelMethod(GetClassMethod("setLevel", "(Lio/sentry/SentryLevel;)V"))
	, GetLevelMethod(GetClassMethod("getLevel", "()Lio/sentry/SentryLevel;"))
{
}

SentryEventAndroid::SentryEventAndroid(jobject event)
	: SentryEventAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if(Env->IsInstanceOf(event, Class))
	{
		// Remove default object's global reference before re-assigning Object field
		Env->DeleteGlobalRef(Object);
		Object = Env->NewGlobalRef(event);
	}
}

FName SentryEventAndroid::GetClassName()
{
	return FName("io/sentry/SentryEvent");
}

void SentryEventAndroid::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, SentryConvertorsAndroid::SentryMessageToNative(message));
}

FString SentryEventAndroid::GetMessage() const
{
	auto message = NewSentryScopedJavaObject(const_cast<SentryEventAndroid*>(this)->CallMethod<jobject>(GetMessageMethod));
	return SentryConvertorsAndroid::SentryMessageToUnreal(*message);
}

void SentryEventAndroid::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, SentryConvertorsAndroid::SentryLevelToNative(level));
}

ESentryLevel SentryEventAndroid::GetLevel() const
{
	auto level = NewSentryScopedJavaObject(const_cast<SentryEventAndroid*>(this)->CallMethod<jobject>(GetLevelMethod));
	return SentryConvertorsAndroid::SentryLevelToUnreal(*level);
}