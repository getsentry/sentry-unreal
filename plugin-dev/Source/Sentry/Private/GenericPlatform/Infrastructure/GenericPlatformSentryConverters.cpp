// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryConverters.h"

#include "SentryDefines.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if USE_SENTRY_NATIVE

sentry_level_e FGenericPlatformSentryConverters::SentryLevelToNative(ESentryLevel level)
{
	sentry_level_e Level = {};

	switch (level)
	{
	case ESentryLevel::Debug:
		Level = SENTRY_LEVEL_DEBUG;
		break;
	case ESentryLevel::Info:
		Level = SENTRY_LEVEL_INFO;
		break;
	case ESentryLevel::Warning:
		Level = SENTRY_LEVEL_WARNING;
		break;
	case ESentryLevel::Error:
		Level = SENTRY_LEVEL_ERROR;
		break;
	case ESentryLevel::Fatal:
		Level = SENTRY_LEVEL_FATAL;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return Level;
}

sentry_value_t FGenericPlatformSentryConverters::StringMapToNative(const TMap<FString, FString>& map)
{
	sentry_value_t nativeValue = sentry_value_new_object();

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		sentry_value_set_by_key(nativeValue, TCHAR_TO_ANSI(*it.Key()), sentry_value_new_string(TCHAR_TO_ANSI(*it.Value())));
	}

	return nativeValue;
}

sentry_value_t FGenericPlatformSentryConverters::StringArrayToNative(const TArray<FString>& array)
{
	sentry_value_t sentryArray = sentry_value_new_list();

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		const FString& ArrayItem = *it;
		sentry_value_append(sentryArray, sentry_value_new_string(TCHAR_TO_ANSI(*ArrayItem)));
	}

	return sentryArray;
}

sentry_value_t FGenericPlatformSentryConverters::VariantToNative(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return sentry_value_new_int32(variant.GetValue<int32>());
	case ESentryVariantType::Float:
		return sentry_value_new_double(variant.GetValue<float>());
	case ESentryVariantType::Bool:
		return sentry_value_new_bool(variant.GetValue<bool>());
	case ESentryVariantType::String:
		return sentry_value_new_string(TCHAR_TO_ANSI(*variant.GetValue<FString>()));
	case ESentryVariantType::Array:
		return VariantArrayToNative(variant.GetValue<TArray<FSentryVariant>>());
	case ESentryVariantType::Map:
		return VariantMapToNative(variant.GetValue<TMap<FString, FSentryVariant>>());
	default:
		return sentry_value_new_null();
	}
}

sentry_value_t FGenericPlatformSentryConverters::VariantArrayToNative(const TArray<FSentryVariant>& array)
{
	sentry_value_t sentryArray = sentry_value_new_list();

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		sentry_value_append(sentryArray, VariantToNative(*it));
	}

	return sentryArray;
}

sentry_value_t FGenericPlatformSentryConverters::VariantMapToNative(const TMap<FString, FSentryVariant>& map)
{
	sentry_value_t sentryObject = sentry_value_new_object();

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		sentry_value_set_by_key(sentryObject, TCHAR_TO_ANSI(*it.Key()), VariantToNative(it.Value()));
	}

	return sentryObject;
}

sentry_value_t FGenericPlatformSentryConverters::AddressToNative(uint64 address)
{
	char buffer[32];
	size_t written = (size_t)snprintf(buffer, sizeof(buffer), "0x%llx", (unsigned long long)address);
	if (written >= sizeof(buffer))
	{
		return sentry_value_new_null();
	}
	buffer[written] = '\0';
	return sentry_value_new_string(buffer);
}

sentry_value_t FGenericPlatformSentryConverters::CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack)
{
	int32 framesCount = callstack.Num();

	sentry_value_t frames = sentry_value_new_list();
	for (int i = 0; i < framesCount; ++i)
	{
		sentry_value_t frame = sentry_value_new_object();
		sentry_value_set_by_key(frame, "instruction_addr", AddressToNative(callstack[framesCount - i - 1].ProgramCounter));
		sentry_value_append(frames, frame);
	}

	sentry_value_t stacktrace = sentry_value_new_object();
	sentry_value_set_by_key(stacktrace, "frames", frames);

	return stacktrace;
}

ESentryLevel FGenericPlatformSentryConverters::SentryLevelToUnreal(sentry_value_t level)
{
	FString levelStr = FString(sentry_value_as_string(level));

	UEnum* Enum = StaticEnum<ESentryLevel>();
	if (!Enum)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
		return ESentryLevel::Debug;
	}
	return static_cast<ESentryLevel>(Enum->GetValueByName(FName(*levelStr)));
}

ESentryLevel FGenericPlatformSentryConverters::SentryLevelToUnreal(sentry_level_t level)
{
	ESentryLevel Level = ESentryLevel::Debug;

	switch (level)
	{
	case SENTRY_LEVEL_DEBUG:
		Level = ESentryLevel::Debug;
		break;
	case SENTRY_LEVEL_INFO:
		Level = ESentryLevel::Info;
		break;
	case SENTRY_LEVEL_WARNING:
		Level = ESentryLevel::Warning;
		break;
	case SENTRY_LEVEL_ERROR:
		Level = ESentryLevel::Error;
		break;
	case SENTRY_LEVEL_FATAL:
		Level = ESentryLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return Level;
}

FSentryVariant FGenericPlatformSentryConverters::VariantToUnreal(sentry_value_t variant)
{
	switch (sentry_value_get_type(variant))
	{
	case SENTRY_VALUE_TYPE_NULL:
		return FSentryVariant();
	case SENTRY_VALUE_TYPE_BOOL:
		return FSentryVariant(static_cast<bool>(sentry_value_is_true(variant)));
	case SENTRY_VALUE_TYPE_INT32:
		return FSentryVariant(sentry_value_as_int32(variant));
	case SENTRY_VALUE_TYPE_DOUBLE:
		return FSentryVariant(static_cast<float>(sentry_value_as_double(variant)));
	case SENTRY_VALUE_TYPE_STRING:
		return FSentryVariant(FString(sentry_value_as_string(variant)));
	case SENTRY_VALUE_TYPE_LIST:
	{
		TArray<FSentryVariant> unrealArray;

		int32 len = sentry_value_get_length(variant);
		for (int32 i = 0; i < len; ++i)
		{
			unrealArray.Add(VariantToUnreal(sentry_value_get_by_index(variant, i)));
		}

		return FSentryVariant(unrealArray);
	}
	case SENTRY_VALUE_TYPE_OBJECT:
	{
		TMap<FString, FSentryVariant> unrealMap;

		FString mapJsonString = FString(sentry_value_to_json(variant));
		if (mapJsonString.IsEmpty() || mapJsonString.Equals(TEXT("null")))
		{
			return unrealMap;
		}

		TSharedPtr<FJsonObject> jsonObject;
		TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(mapJsonString);
		bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonObject);
		if (!bDeserializeSuccess)
		{
			UE_LOG(LogSentrySdk, Error, TEXT("VariantToUnreal failed to deserialize map Json."));
			return unrealMap;
		}

		TArray<FString> keysArr;
		jsonObject->Values.GetKeys(keysArr);

		for (auto it = keysArr.CreateConstIterator(); it; ++it)
		{
			unrealMap.Add(*it, VariantToUnreal(sentry_value_get_by_key(variant, TCHAR_TO_ANSI(**it))));
		}

		return FSentryVariant(unrealMap);
	}
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry variant type used - an empty one will be returned."));
		return FSentryVariant();
	}
}

TMap<FString, FString> FGenericPlatformSentryConverters::StringMapToUnreal(sentry_value_t map)
{
	TMap<FString, FString> unrealMap;

	FString mapJsonString = FString(sentry_value_to_json(map));
	if (mapJsonString.IsEmpty() || mapJsonString.Equals(TEXT("null")))
	{
		return unrealMap;
	}

	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(mapJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonObject);
	if (!bDeserializeSuccess)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("StringMapToUnreal failed to deserialize map Json."));
		return unrealMap;
	}

	TArray<FString> keysArr;
	jsonObject->Values.GetKeys(keysArr);

	for (auto it = keysArr.CreateConstIterator(); it; ++it)
	{
		unrealMap.Add(*it, jsonObject->GetStringField(*it));
	}

	return unrealMap;
}

TArray<FString> FGenericPlatformSentryConverters::StringArrayToUnreal(sentry_value_t array)
{
	TArray<FString> unrealArray;

	FString arrayJsonString = FString(sentry_value_to_json(array));
	if (arrayJsonString.IsEmpty() || arrayJsonString.Equals(TEXT("null")))
	{
		return unrealArray;
	}

	TArray<TSharedPtr<FJsonValue>> jsonArray;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(arrayJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonArray);
	if (!bDeserializeSuccess)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("StringArrayToUnreal failed to deserialize array Json."));
		return unrealArray;
	}

	for (auto it = jsonArray.CreateConstIterator(); it; ++it)
	{
		unrealArray.Add(it->Get()->AsString());
	}

	return unrealArray;
}

FString FGenericPlatformSentryConverters::SentryLevelToString(ESentryLevel level)
{
	const UEnum* EnumPtr = StaticEnum<ESentryLevel>();

	if (!EnumPtr)
	{
		return FString();
	}

	FString ValueStr = EnumPtr->GetNameByValue(static_cast<int64>(level)).ToString();
	FString Result = ValueStr.Replace(*FString::Printf(TEXT("%s::"), TEXT("ESentryLevel")), TEXT("")).ToLower();

	return Result;
}

TArray<uint8> FGenericPlatformSentryConverters::SentryEnvelopeToByteArray(sentry_envelope_t* envelope)
{
	size_t size;
	ANSICHAR* serializedEnvelopeStr = sentry_envelope_serialize(envelope, &size);

	TArray<uint8> envelopeData = TArray<uint8>(reinterpret_cast<uint8*>(serializedEnvelopeStr), size);

	sentry_string_free(serializedEnvelopeStr);

	return envelopeData;
}

ELogVerbosity::Type FGenericPlatformSentryConverters::SentryLevelToLogVerbosity(sentry_level_t level)
{
	ELogVerbosity::Type LogVerbosity = ELogVerbosity::Error;

	switch (level)
	{
	case SENTRY_LEVEL_DEBUG:
		LogVerbosity = ELogVerbosity::Verbose;
		break;
	case SENTRY_LEVEL_INFO:
		LogVerbosity = ELogVerbosity::Log;
		break;
	case SENTRY_LEVEL_WARNING:
		LogVerbosity = ELogVerbosity::Warning;
		break;
	case SENTRY_LEVEL_ERROR:
		LogVerbosity = ELogVerbosity::Error;
		break;
	case SENTRY_LEVEL_FATAL:
		LogVerbosity = ELogVerbosity::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Error will be returned."));
	}

	return LogVerbosity;
}

#endif
