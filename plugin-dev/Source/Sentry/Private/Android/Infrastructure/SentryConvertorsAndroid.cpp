// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsAndroid.h"
#include "SentryJavaObjectWrapper.h"

#include "SentryScope.h"
#include "SentryId.h"
#include "SentryDefines.h"

#include "Android/SentryScopeAndroid.h"
#include "Android/SentryIdAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJavaEnv.h"

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

	FSentryJavaClass SentryLevelJavaClass = FSentryJavaClass { "io/sentry/SentryLevel", ESentryJavaClassType::External };

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryLevelJavaClass, Env->GetStaticObjectField(levelEnumClass, debugEnumFieldField)));
		break;
	case ESentryLevel::Info:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryLevelJavaClass, Env->GetStaticObjectField(levelEnumClass, infoEnumFieldField)));
		break;
	case ESentryLevel::Warning:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryLevelJavaClass, Env->GetStaticObjectField(levelEnumClass, warningEnumFieldField)));
		break;
	case ESentryLevel::Error:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryLevelJavaClass, Env->GetStaticObjectField(levelEnumClass, errorEnumFieldField)));
		break;
	case ESentryLevel::Fatal:
		nativeLevel = MakeShareable(new FSentryJavaObjectWrapper(SentryLevelJavaClass, Env->GetStaticObjectField(levelEnumClass, fatalEnumFieldField)));
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Null will be returned."));
	}

	return nativeLevel;
}

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::SentryMessageToNative(const FString& message)
{
	FSentryJavaClass SentryMessageJavaClass = FSentryJavaClass { "io/sentry/protocol/Message", ESentryJavaClassType::External };
	TSharedPtr<FSentryJavaObjectWrapper> NativeMessage = MakeShareable(new FSentryJavaObjectWrapper(SentryMessageJavaClass, "()V"));
	FSentryJavaMethod SetMessageMethod = NativeMessage->GetMethod("setMessage", "(Ljava/lang/String;)V");

	NativeMessage->CallMethod<void>(SetMessageMethod, *FJavaClassObject::GetJString(message));

	return NativeMessage;
}

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::StringArrayToNative(const TArray<FString>& stringArray)
{
	FSentryJavaClass ArrayListJavaClass = FSentryJavaClass { "java/util/ArrayList", ESentryJavaClassType::System };
	TSharedPtr<FSentryJavaObjectWrapper> NativeArrayList = MakeShareable(new FSentryJavaObjectWrapper(ArrayListJavaClass, "()V"));
	FSentryJavaMethod AddMethod = NativeArrayList->GetMethod("add", "(Ljava/lang/Object;)Z");

	for (const auto& string : stringArray)
	{
		NativeArrayList->CallMethod<bool>(AddMethod, *FJavaClassObject::GetJString(string));
	}

	return NativeArrayList;
}

TSharedPtr<FSentryJavaObjectWrapper> SentryConvertorsAndroid::StringMapToNative(const TMap<FString, FString>& stringMap)
{
	FSentryJavaClass HashMapJavaClass = FSentryJavaClass { "java/util/HashMap", ESentryJavaClassType::System };
	TSharedPtr<FSentryJavaObjectWrapper> NativeHashMap = MakeShareable(new FSentryJavaObjectWrapper(HashMapJavaClass, "()V"));
	FSentryJavaMethod PutMethod = NativeHashMap->GetMethod("put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	for (const auto& dataPair : stringMap)
	{
		NativeHashMap->CallObjectMethod<jobject>(PutMethod, *FJavaClassObject::GetJString(dataPair.Key), *FJavaClassObject::GetJString(dataPair.Value));
	}

	return NativeHashMap;
}

jbyteArray SentryConvertorsAndroid::ByteArrayToNative(const TArray<uint8>& byteArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

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

	FSentryJavaClass SentryLevelJavaClass = FSentryJavaClass { "io/sentry/SentryLevel", ESentryJavaClassType::External };
	FSentryJavaObjectWrapper NativeLevel(SentryLevelJavaClass, level);
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

FString SentryConvertorsAndroid::SentryMessageToUnreal(jobject message)
{
	FSentryJavaClass SentryMessageJavaClass = FSentryJavaClass { "io/sentry/protocol/Message", ESentryJavaClassType::External };
	FSentryJavaObjectWrapper NativeMessage(SentryMessageJavaClass, message);
	FSentryJavaMethod GetMessageMethod = NativeMessage.GetMethod("getMessage", "()Ljava/lang/String;");

	return NativeMessage.CallMethod<FString>(GetMessageMethod);
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

	FSentryJavaClass MapJavaClass = FSentryJavaClass { "java/util/Map", ESentryJavaClassType::System };
	FSentryJavaObjectWrapper NativeMap(MapJavaClass, map);
	FSentryJavaMethod EntrySetMethod = NativeMap.GetMethod("entrySet", "()Ljava/util/Set;");

	FSentryJavaClass SetJavaClass = FSentryJavaClass { "java/util/Set", ESentryJavaClassType::System };
	FSentryJavaObjectWrapper NativeSet(SetJavaClass, *NativeMap.CallObjectMethod<jobject>(EntrySetMethod));
	FSentryJavaMethod IteratorMethod = NativeSet.GetMethod("iterator", "()Ljava/util/Iterator;");

	FSentryJavaClass IteratorJavaClass = FSentryJavaClass { "java/util/Iterator", ESentryJavaClassType::System };
	FSentryJavaObjectWrapper NativeIterator(IteratorJavaClass, *NativeSet.CallObjectMethod<jobject>(IteratorMethod));
	FSentryJavaMethod HasNextMethod = NativeIterator.GetMethod("hasNext", "()Z");
	FSentryJavaMethod NextMethod = NativeIterator.GetMethod("next", "()Ljava/lang/Object;");

	while(NativeIterator.CallMethod<bool>(HasNextMethod))
	{
		FSentryJavaClass MapEntryJavaClass = FSentryJavaClass { "java/util/Map$Entry", ESentryJavaClassType::System };
		FSentryJavaObjectWrapper NativeMapEntry(MapEntryJavaClass, *NativeIterator.CallObjectMethod<jobject>(NextMethod));
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

	FSentryJavaClass ListJavaClass = FSentryJavaClass { "java/util/List", ESentryJavaClassType::System };
	FSentryJavaObjectWrapper NativeList(ListJavaClass, stringList);
	FSentryJavaMethod ToArrayMethod = NativeList.GetMethod("toArray", "()[Ljava/lang/Object;");
	FSentryJavaMethod SizeMethod = NativeList.GetMethod("size", "()I");

	auto objectArray = NativeList.CallObjectMethod<jobjectArray>(ToArrayMethod);

	int length = NativeList.CallMethod<int>(SizeMethod);

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

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

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jbyte* javaByte = Env->GetByteArrayElements(byteArray, 0);

	int length = Env->GetArrayLength(byteArray);

	for (int i = 0; i < length; i++)
	{
		result.Add(javaByte[i]);
	}

	return result;
}