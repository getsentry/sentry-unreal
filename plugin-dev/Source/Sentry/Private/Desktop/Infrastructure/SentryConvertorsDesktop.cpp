// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsDesktop.h"
#include "SentryId.h"
#include "SentryDefines.h"

#include "Desktop/SentryIdDesktop.h"

#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

#if USE_SENTRY_NATIVE

sentry_level_e SentryConvertorsDesktop::SentryLevelToNative(ESentryLevel level)
{
	sentry_level_e desktopLevel = {};

	switch (level)
	{
	case ESentryLevel::Debug:
		desktopLevel = SENTRY_LEVEL_DEBUG;
		break;
	case ESentryLevel::Info:
		desktopLevel = SENTRY_LEVEL_INFO;
		break;
	case ESentryLevel::Warning:
		desktopLevel = SENTRY_LEVEL_WARNING;
		break;
	case ESentryLevel::Error:
		desktopLevel = SENTRY_LEVEL_ERROR;
		break;
	case ESentryLevel::Fatal:
		desktopLevel = SENTRY_LEVEL_FATAL;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return desktopLevel;
}

sentry_value_t SentryConvertorsDesktop::StringMapToNative(const TMap<FString, FString>& map)
{
	sentry_value_t sentryObject = sentry_value_new_object();

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		sentry_value_set_by_key(sentryObject, TCHAR_TO_ANSI(*it.Key()), sentry_value_new_string(TCHAR_TO_ANSI(*it.Value())));
	}

	return sentryObject;
}

ESentryLevel SentryConvertorsDesktop::SentryLevelToUnreal(sentry_value_t level)
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

USentryId* SentryConvertorsDesktop::SentryIdToUnreal(sentry_uuid_t id)
{
	TSharedPtr<SentryIdDesktop> idNativeImpl = MakeShareable(new SentryIdDesktop(id));
	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);
	return unrealId;
}

TMap<FString, FString> SentryConvertorsDesktop::StringMapToUnreal(sentry_value_t map)
{
	TMap<FString, FString> unrealMap;

	FString mapJsonString = FString(sentry_value_to_json(map));

	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(mapJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonObject);
	if (!bDeserializeSuccess) {
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

FString SentryConvertorsDesktop::SentryLevelToString(ESentryLevel level)
{
	const UEnum* EnumPtr = StaticEnum<ESentryLevel>();

	if (!EnumPtr)
	{
		return FString();
	}

	FString ValueStr = EnumPtr->GetNameByValue(static_cast<int64>(level)).ToString();
	FString Result = ValueStr.Replace(*FString::Printf(TEXT("%s::"), TEXT("ESentryLevel")), TEXT(""));

	return Result;
}

#endif
