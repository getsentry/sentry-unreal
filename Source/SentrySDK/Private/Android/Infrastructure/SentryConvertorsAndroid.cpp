// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"
#include "SentryScope.h"
#include "Android/SentryScopeAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

jobject SentryConvertorsAndroid::SentryLevelToNative(ESentryLevel level)
{
	jobject nativeLevel;

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();
	
	jclass levelEnumClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/SentryLevel");

	jfieldID debugEnumFieldField = Env->GetStaticFieldID(levelEnumClass, "DEBUG", "Lio/sentry/SentryLevel;");
	jfieldID infoEnumFieldField = Env->GetStaticFieldID(levelEnumClass, "INFO", "Lio/sentry/SentryLevel;");
	jfieldID warningEnumFieldField = Env->GetStaticFieldID(levelEnumClass, "WARNING", "Lio/sentry/SentryLevel;");
	jfieldID errorEnumFieldField = Env->GetStaticFieldID(levelEnumClass, "ERROR", "Lio/sentry/SentryLevel;");
	jfieldID fatalEnumFieldField = Env->GetStaticFieldID(levelEnumClass, "FATAL", "Lio/sentry/SentryLevel;");

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, debugEnumFieldField);;
		break;
	case ESentryLevel::Info:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, infoEnumFieldField);;
		break;
	case ESentryLevel::Warning:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, warningEnumFieldField);;
		break;
	case ESentryLevel::Error:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, errorEnumFieldField);;
		break;
	case ESentryLevel::Fatal:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, fatalEnumFieldField);;
		break;
	}

	return nativeLevel;
}

jobject SentryConvertorsAndroid::SentryMessageToNative(FString message)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jclass messageClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/protocol/Message");
	jmethodID messageCtor = Env->GetMethodID(messageClass, "<init>", "()V");
	jobject messageObject = Env->NewObject(messageClass, messageCtor);
	jmethodID setMessageMethod = Env->GetMethodID(messageClass, "setMessage", "(Ljava/lang/String;)V");

	Env->CallVoidMethod(messageObject, setMessageMethod, StringToNative(message));

	return messageObject;
}

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

USentryScope* SentryConvertorsAndroid::SentryScopeToUnreal(jobject scope)
{
	TSharedPtr<SentryScopeAndroid> scopeNativeImpl = MakeShareable(new SentryScopeAndroid(scope));
	USentryScope* unrealScope = NewObject<USentryScope>();
	unrealScope->InitWithNativeImplAndroid(scopeNativeImpl);
	return unrealScope;
}
