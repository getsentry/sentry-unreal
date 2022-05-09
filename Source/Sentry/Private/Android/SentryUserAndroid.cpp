// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserAndroid.h"

#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryUserAndroid::SentryUserAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass userClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/protocol/User");
	jmethodID userCtor = Env->GetMethodID(userClass, "<init>", "()V");
	jobject eventObject= Env->NewObject(userClass, userCtor);
	UserAndroid = Env->NewGlobalRef(eventObject);
}

SentryUserAndroid::SentryUserAndroid(jobject user)
{
	UserAndroid = user;
}

SentryUserAndroid::~SentryUserAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(UserAndroid);
}

jobject SentryUserAndroid::GetNativeObject()
{
	return UserAndroid;
}

void SentryUserAndroid::SetEmail(const FString& email)
{
	SentryMethodCallAndroid::CallVoidMethod(UserAndroid, "setEmail", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(email));
}

FString SentryUserAndroid::GetEmail() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserAndroid, "getEmail", "()Ljava/lang/String;");
}

void SentryUserAndroid::SetId(const FString& id)
{
	SentryMethodCallAndroid::CallVoidMethod(UserAndroid, "setId", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(id));
}

FString SentryUserAndroid::GetId() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserAndroid, "getId", "()Ljava/lang/String;");
}

void SentryUserAndroid::SetUsername(const FString& username)
{
	SentryMethodCallAndroid::CallVoidMethod(UserAndroid, "setUsername", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(username));
}

FString SentryUserAndroid::GetUsername() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserAndroid, "getUsername", "()Ljava/lang/String;");
}

void SentryUserAndroid::SetIpAddress(const FString& ipAddress)
{
	SentryMethodCallAndroid::CallVoidMethod(UserAndroid, "setIpAddress", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(ipAddress));
}

FString SentryUserAndroid::GetIpAddress() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserAndroid, "getIpAddress", "()Ljava/lang/String;");
}

void SentryUserAndroid::SetData(const TMap<FString, FString>& data)
{
	SentryMethodCallAndroid::CallVoidMethod(UserAndroid, "setOthers", "(Ljava/util/Map;)V",
		SentryConvertorsAndroid::StringMapToNative(data));
}

TMap<FString, FString> SentryUserAndroid::GetData()
{
	jobject data = SentryMethodCallAndroid::CallObjectMethod(UserAndroid, "getOthers", "()Ljava/util/Map;");
	return SentryConvertorsAndroid::StringMapToUnreal(data);
}
