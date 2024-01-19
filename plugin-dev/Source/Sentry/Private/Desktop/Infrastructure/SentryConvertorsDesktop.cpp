// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsDesktop.h"
#include "SentryId.h"
#include "SentryTransaction.h"
#include "SentrySpan.h"
#include "SentryDefines.h"

#include "Desktop/SentryIdDesktop.h"
#include "Desktop/SentryTransactionDesktop.h"
#include "Desktop/SentrySpanDesktop.h"

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

sentry_value_t SentryConvertorsDesktop::StringArrayToNative(const TArray<FString>& array)
{
	sentry_value_t sentryArray = sentry_value_new_list();

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		const FString& ArrayItem = *it;
		sentry_value_append(sentryArray, sentry_value_new_string(TCHAR_TO_ANSI(*ArrayItem)));
	}

	return sentryArray;
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

ESentryLevel SentryConvertorsDesktop::SentryLevelToUnreal(sentry_level_t level)
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
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));;
	}

	return Level;
}

USentryId* SentryConvertorsDesktop::SentryIdToUnreal(sentry_uuid_t id)
{
	TSharedPtr<SentryIdDesktop> idNativeImpl = MakeShareable(new SentryIdDesktop(id));
	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);
	return unrealId;
}

USentryTransaction* SentryConvertorsDesktop::SentryTransactionToUnreal(sentry_transaction_t* transaction)
{
	TSharedPtr<SentryTransactionDesktop> transactionNativeImpl = MakeShareable(new SentryTransactionDesktop(transaction));
	USentryTransaction* unrealTransaction = NewObject<USentryTransaction>();
	unrealTransaction->InitWithNativeImpl(transactionNativeImpl);
	return unrealTransaction;
}

USentrySpan* SentryConvertorsDesktop::SentrySpanToUnreal(sentry_span_t* span)
{
	TSharedPtr<SentrySpanDesktop> spanNativeImpl = MakeShareable(new SentrySpanDesktop(span));
	USentrySpan* unrealSpan = NewObject<USentrySpan>();
	unrealSpan->InitWithNativeImpl(spanNativeImpl);
	return unrealSpan;
}

TMap<FString, FString> SentryConvertorsDesktop::StringMapToUnreal(sentry_value_t map)
{
	TMap<FString, FString> unrealMap;

	FString mapJsonString = FString(sentry_value_to_json(map));
	if(mapJsonString.IsEmpty() || mapJsonString.Equals(TEXT("null")))
	{
		return unrealMap;
	}

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

TArray<FString> SentryConvertorsDesktop::StringArrayToUnreal(sentry_value_t array)
{
	TArray<FString> unrealArray;

	FString arrayJsonString = FString(sentry_value_to_json(array));
	if(arrayJsonString.IsEmpty() || arrayJsonString.Equals(TEXT("null")))
	{
		return unrealArray;
	}

	TArray<TSharedPtr<FJsonValue>> jsonArray;
	TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(arrayJsonString);
	bool bDeserializeSuccess = FJsonSerializer::Deserialize(jsonReader, jsonArray);
	if (!bDeserializeSuccess) {
		UE_LOG(LogSentrySdk, Error, TEXT("StringArrayToUnreal failed to deserialize array Json."));
		return unrealArray;
	}

	for (auto it = jsonArray.CreateConstIterator(); it; ++it)
	{
		unrealArray.Add(it->Get()->AsString());
	}

	return unrealArray;
}

FString SentryConvertorsDesktop::SentryLevelToString(ESentryLevel level)
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

TArray<uint8> SentryConvertorsDesktop::SentryEnvelopeToByteArray(sentry_envelope_t* envelope)
{
	size_t size;
	ANSICHAR* serializedEnvelopeStr = sentry_envelope_serialize(envelope, &size);

	TArray<uint8> envelopeData = TArray<uint8>(reinterpret_cast<uint8*>(serializedEnvelopeStr), size);

	sentry_string_free(serializedEnvelopeStr);

	return envelopeData;
}

#endif
