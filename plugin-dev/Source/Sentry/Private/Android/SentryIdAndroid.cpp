// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIdAndroid.h"

#include "Android/AndroidApplication.h"

SentryIdAndroid::SentryIdAndroid()
	: FJavaClassObject(GetClassName(), "()V")
	, ToStringMethod(GetClassMethod("toString", "()Ljava/lang/String;"))
{
}

SentryIdAndroid::SentryIdAndroid(jobject id)
	: SentryIdAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if(Env->IsInstanceOf(id, Class))
	{
		// Remove default object's global reference before re-assigning Object field
		Env->DeleteGlobalRef(Object);
		Object = Env->NewGlobalRef(id);
	}
}

FName SentryIdAndroid::GetClassName()
{
	return FName("io/sentry/protocol/SentryId");
}

FString SentryIdAndroid::ToString() const
{
	return const_cast<SentryIdAndroid*>(this)->CallMethod<FString>(ToStringMethod);
}
