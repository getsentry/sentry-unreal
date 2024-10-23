// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"
#include "SentryJavaObjectWrapper.h"
#include "SentryJavaClasses.h"

#include "SentryDefines.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJavaEnv.h"

#include "Dom/JsonValue.h"

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::SentryLevelToNative(ESentryLevel level)
{
	TSharedPtr<FSentryJavaObjectWrapper> nativeLevel = nullptr;

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
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, Env->GetStaticObjectField(levelEnumClass, debugEnumFieldField)));
		break;
	case ESentryLevel::Info:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, Env->GetStaticObjectField(levelEnumClass, infoEnumFieldField)));
		break;
	case ESentryLevel::Warning:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, Env->GetStaticObjectField(levelEnumClass, warningEnumFieldField)));
		break;
	case ESentryLevel::Error:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, Env->GetStaticObjectField(levelEnumClass, errorEnumFieldField)));
		break;
	case ESentryLevel::Fatal:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, Env->GetStaticObjectField(levelEnumClass, fatalEnumFieldField)));
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Null will be returned."));
	}

	return nativeLevel;
}

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::StringArrayToNative(const TArray<FString>& stringArray)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeArrayList = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::ArrayList, "()V"));
	FSentryJavaMethod AddMethod = NativeArrayList->GetMethod("add", "(Ljava/lang/Object;)Z");

	for (const auto& string : stringArray)
	{
		NativeArrayList->CallMethod<bool>(AddMethod, *FSentryJavaObjectWrapper::GetJString(string));
	}

	return NativeArrayList;
}

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::StringMapToNative(const TMap<FString, FString>& stringMap)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeHashMap = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::HashMap, "()V"));
	FSentryJavaMethod PutMethod = NativeHashMap->GetMethod("put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	for (const auto& dataPair : stringMap)
	{
		NativeHashMap->CallObjectMethod<jobject>(PutMethod, *FSentryJavaObjectWrapper::GetJString(dataPair.Key), *FSentryJavaObjectWrapper::GetJString(dataPair.Value));
	}

	return NativeHashMap;
}

jbyteArray SentryConvertorsAndroid::ByteArrayToNative(const TArray<uint8>& byteArray)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jbyteArray javaByteArray = (jbyteArray)Env->NewByteArray(byteArray.Num());

	jbyte* javaByteArrayPtr = (jbyte*)malloc(byteArray.Num() * sizeof(jbyte));

	for (int i = 0; i < byteArray.Num(); i++)
	{
		javaByteArrayPtr[i] = byteArray[i];
	}

	Env->SetByteArrayRegion(javaByteArray, 0, byteArray.Num(), javaByteArrayPtr);

	free(javaByteArrayPtr);

	return javaByteArray;
}

ESentryLevel SentryConvertorsAndroid::SentryLevelToUnreal(jobject level)
{
	ESentryLevel unrealLevel = ESentryLevel::Debug;

	FSentryJavaObjectWrapper NativeLevel(SentryJavaClasses::SentryLevel, level);
	FSentryJavaMethod OrdinalMethod = NativeLevel.GetMethod("ordinal", "()I");

	int levelValue = NativeLevel.CallMethod<int>(OrdinalMethod);

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

TMap<FString, FString> SentryConvertorsAndroid::StringMapToUnreal(jobject map)
{
	TMap<FString, FString> result;

	FSentryJavaObjectWrapper NativeMap(SentryJavaClasses::Map, map);
	FSentryJavaMethod EntrySetMethod = NativeMap.GetMethod("entrySet", "()Ljava/util/Set;");

	FSentryJavaObjectWrapper NativeSet(SentryJavaClasses::Set, *NativeMap.CallObjectMethod<jobject>(EntrySetMethod));
	FSentryJavaMethod IteratorMethod = NativeSet.GetMethod("iterator", "()Ljava/util/Iterator;");

	FSentryJavaObjectWrapper NativeIterator(SentryJavaClasses::Iterator, *NativeSet.CallObjectMethod<jobject>(IteratorMethod));
	FSentryJavaMethod HasNextMethod = NativeIterator.GetMethod("hasNext", "()Z");
	FSentryJavaMethod NextMethod = NativeIterator.GetMethod("next", "()Ljava/lang/Object;");

	while(NativeIterator.CallMethod<bool>(HasNextMethod))
	{
		FSentryJavaObjectWrapper NativeMapEntry(SentryJavaClasses::MapEntry, *NativeIterator.CallObjectMethod<jobject>(NextMethod));
		FSentryJavaMethod GetKeyMethod = NativeMapEntry.GetMethod("getKey", "()Ljava/lang/Object;");
		FSentryJavaMethod GetValueMethod = NativeMapEntry.GetMethod("getValue", "()Ljava/lang/Object;");

		FString Key = NativeMapEntry.CallMethod<FString>(GetKeyMethod);
		FString Value = NativeMapEntry.CallMethod<FString>(GetValueMethod);

		result.Add(Key, Value);
	}

	return result;
}

TArray<FString> SentryConvertorsAndroid::StringListToUnreal(jobject stringList)
{
	TArray<FString> result;

	FSentryJavaObjectWrapper NativeList(SentryJavaClasses::List, stringList);
	FSentryJavaMethod ToArrayMethod = NativeList.GetMethod("toArray", "()[Ljava/lang/Object;");
	FSentryJavaMethod SizeMethod = NativeList.GetMethod("size", "()I");

	auto objectArray = NativeList.CallObjectMethod<jobjectArray>(ToArrayMethod);

	int length = NativeList.CallMethod<int>(SizeMethod);

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	for (int i = 0; i < length; i++)
	{
		result.Add(FJavaHelper::FStringFromLocalRef(Env, static_cast<jstring>(Env->GetObjectArrayElement(*objectArray, i))));
	}

	return result;
}

TArray<uint8> SentryConvertorsAndroid::ByteArrayToUnreal(jbyteArray byteArray)
{
	TArray<uint8> result;

	if (!byteArray)
	{
		return result;
	}

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jbyte* javaByte = Env->GetByteArrayElements(byteArray, 0);

	int length = Env->GetArrayLength(byteArray);

	for (int i = 0; i < length; i++)
	{
		result.Add(javaByte[i]);
	}

	return result;
}

TArray<TSharedPtr<FJsonValue>> SentryConvertorsAndroid::StrinArrayToJsonArray(const TArray<FString>& stringArray)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;

	for (const FString& stringItem : stringArray)
	{
		jsonArray.Add(MakeShareable(new FJsonValueString(stringItem)));
	}

	return jsonArray;
}
