// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"
#include "SentryScope.h"
#include "Android/SentryScopeAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

jstring SentryConvertorsAndroid::StringToNative(FString string)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();
	jstring local = Env->NewStringUTF(TCHAR_TO_UTF8(*string));
	jstring result = (jstring)Env->NewGlobalRef(local);
	Env->DeleteLocalRef(local);
	return result;
}

jobject SentryConvertorsAndroid::StringArrayToNative(const TArray<FString>& stringArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass listClass = Env->FindClass("java/util/List");
	jmethodID listCtor = Env->GetMethodID(listClass, "<init>", "()V");
	jobject list = Env->NewObject(listClass, listCtor);
	jmethodID addMethod = Env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");

	for (const auto& string : stringArray)
	{
		Env->CallBooleanMethod(list, addMethod, StringToNative(string));
	}

	Env->DeleteLocalRef(listClass);

	return list;
}

jobject SentryConvertorsAndroid::StringMapToNative(const TMap<FString, FString>& stringMap)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass hashMapClass = FJavaWrapper::FindClass(Env, "java/util/HashMap", false);
	jmethodID hasMapCtor = FJavaWrapper::FindMethod(Env, hashMapClass, "<init>", "()V", false);
	jmethodID putMethod = FJavaWrapper::FindMethod(Env, hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;", false);
	jobject hashMap = Env->NewObject(hashMapClass, hasMapCtor);

	for (const auto& dataPair : stringMap)
	{
		jobject key = Env->NewStringUTF(TCHAR_TO_UTF8(*dataPair.Key));
		jobject value = Env->NewStringUTF(TCHAR_TO_UTF8(*dataPair.Value));
		jobject prevValue = Env->CallObjectMethod(hashMap, putMethod, key, value);

		if (prevValue)
		{
			Env->DeleteLocalRef(prevValue);
		}

		Env->DeleteLocalRef(key);
		Env->DeleteLocalRef(value);
	}

	return hashMap;
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

USentryScope* SentryConvertorsAndroid::SentryScopeToUnreal(TSharedPtr<SentryScopeAndroid> scope)
{
	USentryScope* unrealScope = NewObject<USentryScope>();
	unrealScope->InitWithNativeImplAndroid(scope);
	return unrealScope;
}
