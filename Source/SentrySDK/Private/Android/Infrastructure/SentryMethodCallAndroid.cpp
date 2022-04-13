// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryMethodCallAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

void SentryMethodCallAndroid::CallStaticVoidMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = FAndroidApplication::FindJavaClass(ClassName);

	jmethodID Method = FJavaWrapper::FindStaticMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	Env->CallStaticVoidMethodV(Class, Method, Args);
	va_end(Args);

	Env->DeleteLocalRef(Class);
}

FString SentryMethodCallAndroid::CallStaticStringMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = FAndroidApplication::FindJavaClass(ClassName);

	jmethodID Method = FJavaWrapper::FindStaticMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	jstring Return = static_cast<jstring>(Env->CallStaticObjectMethodV(Class, Method, Args));
	va_end(Args);

	const char* UTFString = Env->GetStringUTFChars(Return, nullptr);
	FString Result(UTF8_TO_TCHAR(UTFString));
	Env->ReleaseStringUTFChars(Return, UTFString);

	Env->DeleteLocalRef(Class);

	return Result;
}

jobject SentryMethodCallAndroid::CallStaticObjectMethod(const ANSICHAR* ClassName, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = FAndroidApplication::FindJavaClass(ClassName);

	jmethodID Method = FJavaWrapper::FindStaticMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	jobject Result = Env->CallStaticObjectMethodV(Class, Method, Args);
	va_end(Args);

	Env->DeleteLocalRef(Class);

	return Result;
}

void SentryMethodCallAndroid::CallVoidMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = Env->GetObjectClass(object);

	jmethodID Method = FJavaWrapper::FindMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	Env->CallVoidMethodV(object, Method, Args);
	va_end(Args);

	Env->DeleteLocalRef(Class);
}

FString SentryMethodCallAndroid::CallStringMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = Env->GetObjectClass(object);

	jmethodID Method = FJavaWrapper::FindMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	jstring Return = static_cast<jstring>(Env->CallObjectMethodV(object, Method, Args));
	va_end(Args);

	if (Return == nullptr)
	{
		return FString();
	}

	const char* UTFString = Env->GetStringUTFChars(Return, nullptr);
	FString Result(UTF8_TO_TCHAR(UTFString));
	Env->ReleaseStringUTFChars(Return, UTFString);

	Env->DeleteLocalRef(Class);

	return Result;
}

jobject SentryMethodCallAndroid::CallObjectMethod(jobject object, const ANSICHAR* MethodName, const ANSICHAR* MethodSignature, ...)
{
	bool bIsOptional = false;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass Class = Env->GetObjectClass(object);

	jmethodID Method = FJavaWrapper::FindMethod(Env, Class, MethodName, MethodSignature, bIsOptional);

	va_list Args;
	va_start(Args, MethodSignature);
	jobject Result = Env->CallObjectMethodV(object, Method, Args);
	va_end(Args);

	Env->DeleteLocalRef(Class);

	return Result;
}
