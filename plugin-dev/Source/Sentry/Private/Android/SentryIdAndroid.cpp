// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdAndroid.h"

#include "Infrastructure/SentryMethodCallAndroid.h"

SentryIdAndroid::SentryIdAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass idClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/protocol/SentryId");
	jmethodID idCtor = Env->GetMethodID(idClass, "<init>", "()V");
	jobject idObject= Env->NewObject(idClass, idCtor);
	IdAndroid = Env->NewGlobalRef(idObject);
}

SentryIdAndroid::SentryIdAndroid(jobject id)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	IdAndroid = Env->NewGlobalRef(id);
}

SentryIdAndroid::~SentryIdAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(IdAndroid);
}

jobject SentryIdAndroid::GetNativeObject()
{
	return IdAndroid;
}

FString SentryIdAndroid::ToString() const
{
	return SentryMethodCallAndroid::CallStringMethod(IdAndroid, "toString", "()Ljava/lang/String;");
}