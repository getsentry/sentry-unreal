// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryConverters.h"
#include "AndroidSentryJavaClasses.h"
#include "AndroidSentryJavaObjectWrapper.h"

#include "SentryDefines.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJavaEnv.h"

#include "Dom/JsonValue.h"

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::SentryLevelToNative(ESentryLevel level)
{
	jobject nativeLevel = SentryJavaClasses::GetCachedSentryLevelRef(level);

	if (!nativeLevel)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry level value used. Null will be returned."));
		return nullptr;
	}

	return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLevel, nativeLevel));
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::StringArrayToNative(const TArray<FString>& stringArray)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeArrayList = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::ArrayList, "()V"));
	FSentryJavaMethod AddMethod = NativeArrayList->GetMethod("add", "(Ljava/lang/Object;)Z");

	for (const auto& string : stringArray)
	{
		NativeArrayList->CallMethod<bool>(AddMethod, *FSentryJavaObjectWrapper::GetJString(string));
	}

	return NativeArrayList;
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::StringMapToNative(const TMap<FString, FString>& stringMap)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeHashMap = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::HashMap, "()V"));
	FSentryJavaMethod PutMethod = NativeHashMap->GetMethod("put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	for (const auto& dataPair : stringMap)
	{
		NativeHashMap->CallObjectMethod<jobject>(PutMethod, *FSentryJavaObjectWrapper::GetJString(dataPair.Key), *FSentryJavaObjectWrapper::GetJString(dataPair.Value));
	}

	return NativeHashMap;
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::VariantToNative(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::Integer, "(I)V", variant.GetValue<int32>()));
	case ESentryVariantType::Float:
		return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::Float, "(F)V", variant.GetValue<float>()));
	case ESentryVariantType::Bool:
		return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::Boolean, "(Z)V", variant.GetValue<bool>()));
	case ESentryVariantType::String:
		return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::String, "(Ljava/lang/String;)V", *FSentryJavaObjectWrapper::GetJString(variant.GetValue<FString>())));
	case ESentryVariantType::Array:
		return VariantArrayToNative(variant.GetValue<TArray<FSentryVariant>>());
	case ESentryVariantType::Map:
		return VariantMapToNative(variant.GetValue<TMap<FString, FSentryVariant>>());
	default:
		return nullptr;
	}
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::VariantArrayToNative(const TArray<FSentryVariant>& variantArray)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeArrayList = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::ArrayList, "()V"));
	FSentryJavaMethod AddMethod = NativeArrayList->GetMethod("add", "(Ljava/lang/Object;)Z");

	for (const FSentryVariant& Elem : variantArray)
	{
		TSharedPtr<FSentryJavaObjectWrapper> NativeVariant = VariantToNative(Elem);
		if (NativeVariant.IsValid())
		{
			NativeArrayList->CallMethod<bool>(AddMethod, NativeVariant->GetJObject());
		}
	}

	return NativeArrayList;
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap)
{
	TSharedPtr<FSentryJavaObjectWrapper> NativeHashMap = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::HashMap, "()V"));
	FSentryJavaMethod PutMethod = NativeHashMap->GetMethod("put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	for (const auto& dataPair : variantMap)
	{
		TSharedPtr<FSentryJavaObjectWrapper> NativeVariant = VariantToNative(dataPair.Value);
		if (NativeVariant.IsValid())
		{
			NativeHashMap->CallObjectMethod<jobject>(PutMethod, *FSentryJavaObjectWrapper::GetJString(dataPair.Key), NativeVariant->GetJObject());
		}
	}

	return NativeHashMap;
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::TimestampToNative(int64 timestamp)
{
	return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLongDate, "(J)V", (jlong)(timestamp * 1000)));
}

FSentryScopedJavaObject<jbyteArray> FAndroidSentryConverters::ByteArrayToNative(const TArray<uint8>& byteArray)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	jbyteArray javaByteArray = Env->NewByteArray(byteArray.Num());

	Env->SetByteArrayRegion(javaByteArray, 0, byteArray.Num(), reinterpret_cast<const jbyte*>(byteArray.GetData()));

	return NewSentryScopedJavaObject(Env, javaByteArray);
}

ESentryLevel FAndroidSentryConverters::SentryLevelToUnreal(jobject level)
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
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry level value used. Debug will be returned."));
	}

	return unrealLevel;
}

TSharedPtr<FSentryJavaObjectWrapper> FAndroidSentryConverters::SentryLogLevelToNative(ESentryLogLevel level)
{
	jobject nativeLevel = SentryJavaClasses::GetCachedSentryLogLevelRef(level);

	if (!nativeLevel)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry level value used. Debug will be returned."));
		nativeLevel = SentryJavaClasses::GetCachedSentryLogLevelRef(ESentryLogLevel::Debug);
	}

	return MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::SentryLogLevel, nativeLevel));
}

ESentryLogLevel FAndroidSentryConverters::SentryLogLevelToUnreal(jobject level)
{
	ESentryLogLevel unrealLevel = ESentryLogLevel::Debug;

	FSentryJavaObjectWrapper NativeLevel(SentryJavaClasses::SentryLogLevel, level);
	FSentryJavaMethod OrdinalMethod = NativeLevel.GetMethod("ordinal", "()I");

	int levelValue = NativeLevel.CallMethod<int>(OrdinalMethod);

	// See SentryLogLevel.java for the ordinal mapping
	switch (levelValue)
	{
	case 0:
		unrealLevel = ESentryLogLevel::Trace;
		break;
	case 1:
		unrealLevel = ESentryLogLevel::Debug;
		break;
	case 2:
		unrealLevel = ESentryLogLevel::Info;
		break;
	case 3:
		unrealLevel = ESentryLogLevel::Warning;
		break;
	case 4:
		unrealLevel = ESentryLogLevel::Error;
		break;
	case 5:
		unrealLevel = ESentryLogLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry log level value used. Debug will be returned."));
	}

	return unrealLevel;
}

TMap<FString, FString> FAndroidSentryConverters::StringMapToUnreal(jobject map)
{
	TMap<FString, FString> result;

	if (!map)
	{
		return result;
	}

	FSentryJavaObjectWrapper NativeMap(SentryJavaClasses::Map, map);
	FSentryJavaMethod EntrySetMethod = NativeMap.GetMethod("entrySet", "()Ljava/util/Set;");

	FSentryJavaObjectWrapper NativeSet(SentryJavaClasses::Set, *NativeMap.CallObjectMethod<jobject>(EntrySetMethod));
	FSentryJavaMethod IteratorMethod = NativeSet.GetMethod("iterator", "()Ljava/util/Iterator;");

	FSentryJavaObjectWrapper NativeIterator(SentryJavaClasses::Iterator, *NativeSet.CallObjectMethod<jobject>(IteratorMethod));
	FSentryJavaMethod HasNextMethod = NativeIterator.GetMethod("hasNext", "()Z");
	FSentryJavaMethod NextMethod = NativeIterator.GetMethod("next", "()Ljava/lang/Object;");

	while (NativeIterator.CallMethod<bool>(HasNextMethod))
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

TArray<FString> FAndroidSentryConverters::StringListToUnreal(jobject stringList)
{
	TArray<FString> result;

	if (!stringList)
	{
		return result;
	}

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

TArray<uint8> FAndroidSentryConverters::ByteArrayToUnreal(jbyteArray byteArray)
{
	TArray<uint8> result;

	if (!byteArray)
	{
		return result;
	}

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	int length = Env->GetArrayLength(byteArray);

	result.SetNumUninitialized(length);

	Env->GetByteArrayRegion(byteArray, 0, length, reinterpret_cast<jbyte*>(result.GetData()));

	return result;
}

FSentryVariant FAndroidSentryConverters::VariantToUnreal(jobject variant)
{
	if (variant == nullptr)
	{
		return FSentryVariant();
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::Integer, variant))
	{
		FSentryJavaObjectWrapper NativeIntVariant(SentryJavaClasses::Integer, variant);
		FSentryJavaMethod IntValueMethod = NativeIntVariant.GetMethod("intValue", "()I");
		return FSentryVariant(NativeIntVariant.CallMethod<int>(IntValueMethod));
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::Float, variant))
	{
		FSentryJavaObjectWrapper NativeFloatVariant(SentryJavaClasses::Float, variant);
		FSentryJavaMethod FloatValueMethod = NativeFloatVariant.GetMethod("floatValue", "()F");
		return FSentryVariant(NativeFloatVariant.CallMethod<float>(FloatValueMethod));
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::Boolean, variant))
	{
		FSentryJavaObjectWrapper NativeBoolVariant(SentryJavaClasses::Boolean, variant);
		FSentryJavaMethod BoolValueMethod = NativeBoolVariant.GetMethod("booleanValue", "()Z");
		return FSentryVariant(NativeBoolVariant.CallMethod<bool>(BoolValueMethod));
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::String, variant))
	{
		FSentryJavaObjectWrapper NativeStringVariant(SentryJavaClasses::String, variant);
		FSentryJavaMethod ToStringMethod = NativeStringVariant.GetMethod("toString", "()Ljava/lang/String;");
		return FSentryVariant(NativeStringVariant.CallMethod<FString>(ToStringMethod));
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::List, variant))
	{
		return FSentryVariant(FAndroidSentryConverters::VariantArrayToUnreal(variant));
	}
	if (FSentryJavaObjectWrapper::IsInstanceOf(SentryJavaClasses::Map, variant))
	{
		return FSentryVariant(FAndroidSentryConverters::VariantMapToUnreal(variant));
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("Couldn't convert Java object to variant. Empty one will be returned."));
	return FSentryVariant();
}

TArray<FSentryVariant> FAndroidSentryConverters::VariantArrayToUnreal(jobject variantArray)
{
	TArray<FSentryVariant> result;

	if (!variantArray)
	{
		return result;
	}

	FSentryJavaObjectWrapper NativeList(SentryJavaClasses::List, variantArray);
	FSentryJavaMethod ToArrayMethod = NativeList.GetMethod("toArray", "()[Ljava/lang/Object;");
	FSentryJavaMethod SizeMethod = NativeList.GetMethod("size", "()I");

	auto objectArray = NativeList.CallObjectMethod<jobjectArray>(ToArrayMethod);

	int length = NativeList.CallMethod<int>(SizeMethod);

	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();

	for (int i = 0; i < length; i++)
	{
		auto element = NewSentryScopedJavaObject(Env, Env->GetObjectArrayElement(*objectArray, i));
		result.Add(FSentryVariant(VariantToUnreal(*element)));
	}

	return result;
}

TMap<FString, FSentryVariant> FAndroidSentryConverters::VariantMapToUnreal(jobject variantMap)
{
	TMap<FString, FSentryVariant> result;

	if (!variantMap)
	{
		return result;
	}

	FSentryJavaObjectWrapper NativeMap(SentryJavaClasses::Map, variantMap);
	FSentryJavaMethod EntrySetMethod = NativeMap.GetMethod("entrySet", "()Ljava/util/Set;");

	FSentryJavaObjectWrapper NativeSet(SentryJavaClasses::Set, *NativeMap.CallObjectMethod<jobject>(EntrySetMethod));
	FSentryJavaMethod IteratorMethod = NativeSet.GetMethod("iterator", "()Ljava/util/Iterator;");

	FSentryJavaObjectWrapper NativeIterator(SentryJavaClasses::Iterator, *NativeSet.CallObjectMethod<jobject>(IteratorMethod));
	FSentryJavaMethod HasNextMethod = NativeIterator.GetMethod("hasNext", "()Z");
	FSentryJavaMethod NextMethod = NativeIterator.GetMethod("next", "()Ljava/lang/Object;");

	while (NativeIterator.CallMethod<bool>(HasNextMethod))
	{
		FSentryJavaObjectWrapper NativeMapEntry(SentryJavaClasses::MapEntry, *NativeIterator.CallObjectMethod<jobject>(NextMethod));
		FSentryJavaMethod GetKeyMethod = NativeMapEntry.GetMethod("getKey", "()Ljava/lang/Object;");
		FSentryJavaMethod GetValueMethod = NativeMapEntry.GetMethod("getValue", "()Ljava/lang/Object;");

		FString Key = NativeMapEntry.CallMethod<FString>(GetKeyMethod);
		FSentryVariant Value = VariantToUnreal(*NativeMapEntry.CallObjectMethod<jobject>(GetValueMethod));

		result.Add(Key, Value);
	}

	return result;
}

TArray<TSharedPtr<FJsonValue>> FAndroidSentryConverters::StrinArrayToJsonArray(const TArray<FString>& stringArray)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;

	for (const FString& stringItem : stringArray)
	{
		jsonArray.Add(MakeShareable(new FJsonValueString(stringItem)));
	}

	return jsonArray;
}
