// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"

#include "Android/AndroidJava.h"

jstring SentryConvertorsAndroid::StringToNative(FString string)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	jstring local = JEnv->NewStringUTF(TCHAR_TO_UTF8(*string));
	jstring result = (jstring)JEnv->NewGlobalRef(local);
	JEnv->DeleteLocalRef(local);
	return result;
}

FString SentryConvertorsAndroid::StringToUnreal(jstring string)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();
	const char* UTFString = Env->GetStringUTFChars(string, nullptr);
	FString Result(UTF8_TO_TCHAR(UTFString));
	Env->ReleaseStringUTFChars(string, UTFString);
	Env->DeleteLocalRef(string);
	return Result;
}
