// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryConverters.h"

#include "SentryDefines.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
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
		sentry_value_set_by_key(nativeValue, TCHAR_TO_UTF8(*it.Key()), sentry_value_new_string(TCHAR_TO_UTF8(*it.Value())));
	}

	return nativeValue;
}

sentry_value_t FGenericPlatformSentryConverters::StringArrayToNative(const TArray<FString>& array)
{
	sentry_value_t sentryArray = sentry_value_new_list();

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		const FString& ArrayItem = *it;
		sentry_value_append(sentryArray, sentry_value_new_string(TCHAR_TO_UTF8(*ArrayItem)));
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
		return sentry_value_new_string(TCHAR_TO_UTF8(*variant.GetValue<FString>()));
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
		sentry_value_set_by_key(sentryObject, TCHAR_TO_UTF8(*it.Key()), VariantToNative(it.Value()));
	}

	return sentryObject;
}

sentry_value_t FGenericPlatformSentryConverters::VariantToAttributeNative(const FSentryVariant& variant)
{
	sentry_value_t value;

	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		value = sentry_value_new_int32(variant.GetValue<int32>());
		break;
	case ESentryVariantType::Float:
		value = sentry_value_new_double(variant.GetValue<float>());
		break;
	case ESentryVariantType::Bool:
		value = sentry_value_new_bool(variant.GetValue<bool>());
		break;
	case ESentryVariantType::String:
		value = sentry_value_new_string(TCHAR_TO_UTF8(*variant.GetValue<FString>()));
		break;
	case ESentryVariantType::Array:
	case ESentryVariantType::Map:
	{
		// For complex types (arrays, maps), convert to JSON string
		TSharedPtr<FJsonValue> jsonValue = VariantToJsonValue(variant);
		if (jsonValue.IsValid())
		{
			FString jsonString;
			TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&jsonString);
			FJsonSerializer::Serialize(jsonValue.ToSharedRef(), TEXT(""), writer);
			value = sentry_value_new_string(TCHAR_TO_UTF8(*jsonString));
		}
		else
		{
			value = sentry_value_new_null();
		}
		break;
	}
	default:
		value = sentry_value_new_null();
		break;
	}

	return sentry_value_new_attribute(value, nullptr);
}

sentry_value_t FGenericPlatformSentryConverters::VariantMapToAttributesNative(const TMap<FString, FSentryVariant>& map)
{
	if (map.Num() == 0)
	{
		return sentry_value_new_null();
	}

	sentry_value_t attributes = sentry_value_new_object();
	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		sentry_value_set_by_key(attributes, TCHAR_TO_UTF8(*it.Key()), VariantToAttributeNative(it.Value()));
	}

	return attributes;
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

sentry_minidump_mode_t FGenericPlatformSentryConverters::MinidumpModeToNative(ESentryMinidumpMode mode)
{
	switch (mode)
	{
	case ESentryMinidumpMode::StackOnly:
		return SENTRY_MINIDUMP_MODE_STACK_ONLY;
	case ESentryMinidumpMode::Smart:
		return SENTRY_MINIDUMP_MODE_SMART;
	case ESentryMinidumpMode::Full:
		return SENTRY_MINIDUMP_MODE_FULL;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown minidump mode value used. Smart will be returned."));
		return SENTRY_MINIDUMP_MODE_SMART;
	}
}

sentry_crash_reporting_mode_t FGenericPlatformSentryConverters::CrashReportingModeToNative(ESentryCrashReportingMode mode)
{
	switch (mode)
	{
	case ESentryCrashReportingMode::Minidump:
		return SENTRY_CRASH_REPORTING_MODE_MINIDUMP;
	case ESentryCrashReportingMode::NativeStackwalking:
		return SENTRY_CRASH_REPORTING_MODE_NATIVE;
	case ESentryCrashReportingMode::NativeStackwalkingWithMinidump:
		return SENTRY_CRASH_REPORTING_MODE_NATIVE_WITH_MINIDUMP;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown crash reporting mode value used. NativeStackwalkingWithMinidump will be returned."));
		return SENTRY_CRASH_REPORTING_MODE_NATIVE_WITH_MINIDUMP;
	}
}

#ifdef USE_SENTRY_SESSION_REPLAY
sentry_value_t FGenericPlatformSentryConverters::ReplayEventToNative(const FSentryReplayInfo& info)
{
	sentry_value_t event = sentry_value_new_object();
	sentry_value_set_by_key(event, "type", sentry_value_new_string("replay_event"));
	sentry_value_set_by_key(event, "replay_type", sentry_value_new_string(TCHAR_TO_UTF8(*info.ReplayType)));
	sentry_value_set_by_key(event, "segment_id", sentry_value_new_int32(info.SegmentId));
	sentry_value_set_by_key(event, "replay_id", sentry_value_new_string(TCHAR_TO_UTF8(*info.ReplayId)));
	sentry_value_set_by_key(event, "event_id", sentry_value_new_string(TCHAR_TO_UTF8(*info.ReplayId)));
	sentry_value_set_by_key(event, "platform", sentry_value_new_string("native"));
	sentry_value_set_by_key(event, "timestamp", sentry_value_new_double(info.EndTimestampSec));
	sentry_value_set_by_key(event, "replay_start_timestamp", sentry_value_new_double(info.StartTimestampSec));
	sentry_value_set_by_key(event, "urls", sentry_value_new_list());

	sentry_value_t errorIds = sentry_value_new_list();
	if (!info.ErrorEventId.IsEmpty())
	{
		sentry_value_append(errorIds, sentry_value_new_string(TCHAR_TO_UTF8(*info.ErrorEventId)));
	}
	sentry_value_set_by_key(event, "error_ids", errorIds);

	return event;
}

sentry_value_t FGenericPlatformSentryConverters::ReplayRecordingToNative(const FSentryReplayInfo& info)
{
	const double tsMs = info.StartTimestampSec * 1000.0;

	sentry_value_t metaData = sentry_value_new_object();
	sentry_value_set_by_key(metaData, "href", sentry_value_new_string(""));
	sentry_value_set_by_key(metaData, "width", sentry_value_new_int32(info.Width));
	sentry_value_set_by_key(metaData, "height", sentry_value_new_int32(info.Height));
	sentry_value_t metaEvent = sentry_value_new_object();
	sentry_value_set_by_key(metaEvent, "type", sentry_value_new_int32(4));
	sentry_value_set_by_key(metaEvent, "timestamp", sentry_value_new_double(tsMs));
	sentry_value_set_by_key(metaEvent, "data", metaData);

	sentry_value_t payload = sentry_value_new_object();
	sentry_value_set_by_key(payload, "segmentId", sentry_value_new_int32(info.SegmentId));
	sentry_value_set_by_key(payload, "size", sentry_value_new_double(static_cast<double>(info.SizeBytes)));
	sentry_value_set_by_key(payload, "duration", sentry_value_new_double(static_cast<double>(info.DurationMs)));
	sentry_value_set_by_key(payload, "encoding", sentry_value_new_string("h264"));
	sentry_value_set_by_key(payload, "container", sentry_value_new_string("mp4"));
	sentry_value_set_by_key(payload, "height", sentry_value_new_int32(info.Height));
	sentry_value_set_by_key(payload, "width", sentry_value_new_int32(info.Width));
	sentry_value_set_by_key(payload, "left", sentry_value_new_int32(0));
	sentry_value_set_by_key(payload, "top", sentry_value_new_int32(0));
	sentry_value_set_by_key(payload, "frameCount", sentry_value_new_int32(info.FrameCount));
	sentry_value_set_by_key(payload, "frameRate", sentry_value_new_int32(info.FrameRate));
	sentry_value_set_by_key(payload, "frameRateType", sentry_value_new_string("variable"));
	sentry_value_t videoData = sentry_value_new_object();
	sentry_value_set_by_key(videoData, "tag", sentry_value_new_string("video"));
	sentry_value_set_by_key(videoData, "payload", payload);
	sentry_value_t videoEvent = sentry_value_new_object();
	sentry_value_set_by_key(videoEvent, "type", sentry_value_new_int32(5));
	sentry_value_set_by_key(videoEvent, "timestamp", sentry_value_new_double(tsMs));
	sentry_value_set_by_key(videoEvent, "data", videoData);

	sentry_value_t recording = sentry_value_new_list();
	sentry_value_append(recording, metaEvent);
	sentry_value_append(recording, videoEvent);
	return recording;
}
#endif // USE_SENTRY_SESSION_REPLAY

ESentryLevel FGenericPlatformSentryConverters::SentryLevelToUnreal(sentry_value_t level)
{
	FString levelStr = FString(UTF8_TO_TCHAR(sentry_value_as_string(level)));

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
		return FSentryVariant(FString(UTF8_TO_TCHAR(sentry_value_as_string(variant))));
	case SENTRY_VALUE_TYPE_LIST:
		return VariantArrayToUnreal(variant);
	case SENTRY_VALUE_TYPE_OBJECT:
		return VariantMapToUnreal(variant);
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry variant type used - an empty one will be returned."));
		return FSentryVariant();
	}
}

TMap<FString, FSentryVariant> FGenericPlatformSentryConverters::VariantMapToUnreal(sentry_value_t map)
{
	TMap<FString, FSentryVariant> unrealMap;

	char* jsonString = sentry_value_to_json(map);
	if (!jsonString)
	{
		return unrealMap;
	}

	FString mapJsonString = FString(UTF8_TO_TCHAR(jsonString));
	if (mapJsonString.IsEmpty() || mapJsonString.Equals(TEXT("null")))
	{
		sentry_string_free(jsonString);
		return unrealMap;
	}

	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(mapJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonObject);
	if (!bDeserializeSuccess)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("VariantToUnreal failed to deserialize map Json."));
		sentry_string_free(jsonString);
		return unrealMap;
	}

	for (const auto& pair : jsonObject->Values)
	{
		unrealMap.Add(FString(*pair.Key), VariantToUnreal(sentry_value_get_by_key(map, TCHAR_TO_UTF8(*pair.Key))));
	}

	sentry_string_free(jsonString);
	return unrealMap;
}

TArray<FSentryVariant> FGenericPlatformSentryConverters::VariantArrayToUnreal(sentry_value_t array)
{
	TArray<FSentryVariant> unrealArray;

	int32 len = sentry_value_get_length(array);
	for (int32 i = 0; i < len; ++i)
	{
		unrealArray.Add(VariantToUnreal(sentry_value_get_by_index(array, i)));
	}

	return unrealArray;
}

TMap<FString, FString> FGenericPlatformSentryConverters::StringMapToUnreal(sentry_value_t map)
{
	TMap<FString, FString> unrealMap;

	char* jsonString = sentry_value_to_json(map);
	if (!jsonString)
	{
		return unrealMap;
	}

	FString mapJsonString = FString(UTF8_TO_TCHAR(jsonString));
	if (mapJsonString.IsEmpty() || mapJsonString.Equals(TEXT("null")))
	{
		sentry_string_free(jsonString);
		return unrealMap;
	}

	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(mapJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonObject);
	if (!bDeserializeSuccess)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("StringMapToUnreal failed to deserialize map Json."));
		sentry_string_free(jsonString);
		return unrealMap;
	}

	for (const auto& pair : jsonObject->Values)
	{
		FString key(*pair.Key);
		unrealMap.Add(key, jsonObject->GetStringField(key));
	}

	sentry_string_free(jsonString);
	return unrealMap;
}

TArray<FString> FGenericPlatformSentryConverters::StringArrayToUnreal(sentry_value_t array)
{
	TArray<FString> unrealArray;

	int32 len = sentry_value_get_length(array);
	for (int32 i = 0; i < len; ++i)
	{
		unrealArray.Add(UTF8_TO_TCHAR(sentry_value_as_string(sentry_value_get_by_index(array, i))));
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

TSharedPtr<FJsonValue> FGenericPlatformSentryConverters::VariantToJsonValue(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return MakeShareable(new FJsonValueNumber(variant.GetValue<int32>()));
	case ESentryVariantType::Float:
		return MakeShareable(new FJsonValueNumber(variant.GetValue<float>()));
	case ESentryVariantType::Bool:
		return MakeShareable(new FJsonValueBoolean(variant.GetValue<bool>()));
	case ESentryVariantType::String:
		return MakeShareable(new FJsonValueString(variant.GetValue<FString>()));
	case ESentryVariantType::Array:
		return VariantArrayToJsonValue(variant.GetValue<TArray<FSentryVariant>>());
	case ESentryVariantType::Map:
		return VariantMapToJsonValue(variant.GetValue<TMap<FString, FSentryVariant>>());
	default:
		return MakeShareable(new FJsonValueNull());
	}
}

TSharedPtr<FJsonValue> FGenericPlatformSentryConverters::VariantArrayToJsonValue(const TArray<FSentryVariant>& array)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		const FSentryVariant& variant = *it;
		TSharedPtr<FJsonValue> jsonValue = VariantToJsonValue(variant);
		if (jsonValue.IsValid())
		{
			jsonArray.Add(jsonValue);
		}
	}

	return MakeShareable(new FJsonValueArray(jsonArray));
}

TSharedPtr<FJsonValue> FGenericPlatformSentryConverters::VariantMapToJsonValue(const TMap<FString, FSentryVariant>& map)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		const FString& key = it.Key();
		const FSentryVariant& variant = it.Value();

		if (variant.GetType() == ESentryVariantType::Empty)
		{
			continue;
		}

		TSharedPtr<FJsonValue> jsonValue = VariantToJsonValue(variant);
		if (jsonValue.IsValid())
		{
			jsonObject->SetField(key, jsonValue);
		}
	}

	return MakeShareable(new FJsonValueObject(jsonObject));
}

#endif
