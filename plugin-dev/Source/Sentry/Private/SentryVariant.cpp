// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryVariant.h"
#include "SentryDefines.h"

template int32 FSentryVariant::GetValue<int32>() const;
template float FSentryVariant::GetValue<float>() const;
template bool FSentryVariant::GetValue<bool>() const;
template FString FSentryVariant::GetValue<FString>() const;
template TArray<FSentryVariant> FSentryVariant::GetValue<TArray<FSentryVariant>>() const;
template TMap<FString, FSentryVariant> FSentryVariant::GetValue<TMap<FString, FSentryVariant>>() const;

FSentryVariant::FSentryVariant()
	: Type(ESentryVariantType::Empty) {}
FSentryVariant::FSentryVariant(int32 InValue)
	: Type(ESentryVariantType::Integer), Value(InValue) {}
FSentryVariant::FSentryVariant(float InValue)
	: Type(ESentryVariantType::Float), Value(InValue) {}
FSentryVariant::FSentryVariant(bool InValue)
	: Type(ESentryVariantType::Bool), Value(InValue) {}
FSentryVariant::FSentryVariant(const FString& InValue)
	: Type(ESentryVariantType::String), Value(InValue) {}
FSentryVariant::FSentryVariant(const TCHAR* InValue)
	: Type(ESentryVariantType::String), Value(InValue) {}
FSentryVariant::FSentryVariant(const TArray<FSentryVariant>& InValue)
	: Type(ESentryVariantType::Array), Value(InValue) {}
FSentryVariant::FSentryVariant(const TMap<FString, FSentryVariant>& InValue)
	: Type(ESentryVariantType::Map), Value(InValue) {}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromInteger(int32 Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromFloat(float Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromBool(bool Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromString(const FString& Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromArray(const TArray<FSentryVariant>& Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::MakeSentryVariantFromMap(const TMap<FString, FSentryVariant>& Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_IntToFSentryVariant(int32 Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_FloatToFSentryVariant(float Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_BoolToFSentryVariant(bool Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_StringToFSentryVariant(const FString& Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_ArrayToFSentryVariant(const TArray<FSentryVariant>& Value)
{
	return FSentryVariant(Value);
}

FSentryVariant USentryVariantHelper::Conv_MapToFSentryVariant(const TMap<FString, FSentryVariant>& Value)
{
	return FSentryVariant(Value);
}

ESentryVariantType USentryVariantHelper::GetType(const FSentryVariant& Variant)
{
	return Variant.GetType();
}

int32 USentryVariantHelper::GetInteger(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::Integer)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain an integer value. Returning default value."));
		return 0;
	}

	return Variant.GetValue<int32>();
}

bool USentryVariantHelper::TryGetInteger(const FSentryVariant& Variant, int32& Value)
{
	if (Variant.GetType() != ESentryVariantType::Integer)
	{
		Value = 0;
		return false;
	}

	Value = Variant.GetValue<int32>();
	return true;
}

float USentryVariantHelper::GetFloat(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::Float)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain a float value. Returning default value."));
		return 0.0f;
	}

	return Variant.GetValue<float>();
}

bool USentryVariantHelper::TryGetFloat(const FSentryVariant& Variant, float& Value)
{
	if (Variant.GetType() != ESentryVariantType::Float)
	{
		Value = 0.0f;
		return false;
	}

	Value = Variant.GetValue<float>();
	return true;
}

bool USentryVariantHelper::GetBool(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::Bool)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain a bool value. Returning default value."));
		return false;
	}

	return Variant.GetValue<bool>();
}

bool USentryVariantHelper::TryGetBool(const FSentryVariant& Variant, bool& Value)
{
	if (Variant.GetType() != ESentryVariantType::Bool)
	{
		Value = false;
		return false;
	}

	Value = Variant.GetValue<bool>();
	return true;
}

FString USentryVariantHelper::GetString(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::String)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain a string value. Returning default value."));
		return FString("");
	}

	return Variant.GetValue<FString>();
}

bool USentryVariantHelper::TryGetString(const FSentryVariant& Variant, FString& Value)
{
	if (Variant.GetType() != ESentryVariantType::String)
	{
		Value = FString("");
		return false;
	}

	Value = Variant.GetValue<FString>();
	return true;
}

TArray<FSentryVariant> USentryVariantHelper::GetArray(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::Array)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain an array value. Returning default value."));
		return TArray<FSentryVariant>();
	}

	return Variant.GetValue<TArray<FSentryVariant>>();
}

bool USentryVariantHelper::TryGetArray(const FSentryVariant& Variant, TArray<FSentryVariant>& Value)
{
	if (Variant.GetType() != ESentryVariantType::Array)
	{
		Value = TArray<FSentryVariant>();
		return false;
	}

	Value = Variant.GetValue<TArray<FSentryVariant>>();
	return true;
}

TMap<FString, FSentryVariant> USentryVariantHelper::GetMap(const FSentryVariant& Variant)
{
	if (Variant.GetType() != ESentryVariantType::Map)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Variant does not contain a map value. Returning default value."));
		return TMap<FString, FSentryVariant>();
	}

	return Variant.GetValue<TMap<FString, FSentryVariant>>();
}

bool USentryVariantHelper::TryGetMap(const FSentryVariant& Variant, TMap<FString, FSentryVariant>& Value)
{
	if (Variant.GetType() != ESentryVariantType::Map)
	{
		Value = TMap<FString, FSentryVariant>();
		return false;
	}

	Value = Variant.GetValue<TMap<FString, FSentryVariant>>();
	return true;
}