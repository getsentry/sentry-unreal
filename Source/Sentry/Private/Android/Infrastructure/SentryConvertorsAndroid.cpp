// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"
#include "SentryScope.h"
#include "SentryId.h"
#include "SentryDefines.h"

#include "Android/SentryScopeAndroid.h"
#include "Android/SentryIdAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
#include "Android/SentryIdAndroid.h"

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
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, debugEnumFieldField);
		break;
	case ESentryLevel::Info:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, infoEnumFieldField);
		break;
	case ESentryLevel::Warning:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, warningEnumFieldField);
		break;
	case ESentryLevel::Error:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, errorEnumFieldField);
		break;
	case ESentryLevel::Fatal:
		nativeLevel = Env->GetStaticObjectField(levelEnumClass, fatalEnumFieldField);
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Null will be returned."));
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

	jclass listClass = Env->FindClass("java/util/ArrayList");
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

ESentryLevel SentryConvertorsAndroid::SentryLevelToUnreal(jobject level)
{
	ESentryLevel unrealLevel = ESentryLevel::Debug;

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jclass levelEnumClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/SentryLevel");
	jmethodID ordinalMethod = FJavaWrapper::FindMethod(Env, levelEnumClass, "ordinal", "()I", false);
	jint levelValue = Env->CallIntMethod(level, ordinalMethod);

	switch (levelValue)
	{
	case 0:
		unrealLevel = ESentryLevel::Debug;
		break;
	case 1:
		unrealLevel = ESentryLevel::Info;
		break;
	case 2:
		unrealLevel = ESentryLevel::Warning;
		break;
	case 3:
		unrealLevel = ESentryLevel::Error;
		break;
	case 4:
		unrealLevel = ESentryLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return unrealLevel;
}

FString SentryConvertorsAndroid::SentryMessageToUnreal(jobject message)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jclass messageClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/protocol/Message");
	jmethodID getMessageMethod = Env->GetMethodID(messageClass, "getMessage", "()Ljava/lang/String;");

	jstring messageStr = static_cast<jstring>(Env->CallObjectMethod(message, getMessageMethod));

	return StringToUnreal(messageStr);
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
	unrealScope->InitWithNativeImpl(scopeNativeImpl);
	return unrealScope;
}

USentryId* SentryConvertorsAndroid::SentryIdToUnreal(jobject id)
{
	TSharedPtr<SentryIdAndroid> idNativeImpl = MakeShareable(new SentryIdAndroid(id));
	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);
	return unrealId;
}

TMap<FString, FString> SentryConvertorsAndroid::StringMapToUnreal(jobject map)
{
	TMap<FString, FString> result;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass mapClass = Env->FindClass("java/util/Map");
	jmethodID entrySetMethod = Env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
	jobject set = Env->CallObjectMethod(map, entrySetMethod);

	jclass setClass = Env->FindClass("java/util/Set");
	jmethodID iteratorMethod = Env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
	jobject iter = Env->CallObjectMethod(set, iteratorMethod);
	
	jclass iteratorClass = Env->FindClass("java/util/Iterator");
	jmethodID hasNextMethod = Env->GetMethodID(iteratorClass, "hasNext", "()Z");
	jmethodID nextMethod = Env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
	
	jclass entryClass = Env->FindClass("java/util/Map$Entry");
	jmethodID getKeyMethod = Env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
	jmethodID getValueMethod = Env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
	
	while (Env->CallBooleanMethod(iter, hasNextMethod))
	{
		jobject entry = Env->CallObjectMethod(iter, nextMethod);

		jstring javaKey = static_cast<jstring>(Env->CallObjectMethod(entry, getKeyMethod));
		jstring javaValue = static_cast<jstring>(Env->CallObjectMethod(entry, getValueMethod));

		FString Key = StringToUnreal(javaKey);
		FString Value = StringToUnreal(javaValue);

		result.Add(Key, Value);

		Env->DeleteLocalRef(entry);
	}

	return result;
}

TArray<FString> SentryConvertorsAndroid::StringListToUnreal(jobject stringList)
{
	TArray<FString> result;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass listClass = Env->FindClass("java/util/List");
	jmethodID toArrayMethod = Env->GetMethodID(listClass, "toArray", "()[Ljava/lang/Object;");

	jobjectArray objectArray = static_cast<jobjectArray>(Env->CallObjectMethod(stringList, toArrayMethod));

	int length = Env->GetArrayLength(objectArray);

	for (int i = 0; i < length; i++)
	{
		jstring javaString = static_cast<jstring>(Env->GetObjectArrayElement(objectArray, i));
		result.Add(StringToUnreal(javaString));
	}

	return result;
}
