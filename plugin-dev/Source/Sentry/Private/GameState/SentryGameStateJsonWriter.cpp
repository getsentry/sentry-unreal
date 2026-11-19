// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateJsonWriter.h"

#include "SentryVariant.h"

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::BeginObject(const TCHAR* Key)
{
	WriteKey(Key);
	Output.AppendChar(TEXT('{'));
	HasElements.Push(false);
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::EndObject()
{
	check(HasElements.Num() > 0);
	HasElements.Pop();
	Output.AppendChar(TEXT('}'));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::BeginArray(const TCHAR* Key)
{
	WriteKey(Key);
	Output.AppendChar(TEXT('['));
	HasElements.Push(false);
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::EndArray()
{
	check(HasElements.Num() > 0);
	HasElements.Pop();
	Output.AppendChar(TEXT(']'));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, const FString& Value)
{
	WriteKey(Key);
	Output.AppendChar(TEXT('"'));
	Output.Append(EscapeString(Value));
	Output.AppendChar(TEXT('"'));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, const TCHAR* Value)
{
	return Write(Key, FString(Value));
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, bool Value)
{
	WriteKey(Key);
	WriteRaw(Value ? TEXT("true") : TEXT("false"));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, int32 Value)
{
	return Write(Key, static_cast<int64>(Value));
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, int64 Value)
{
	WriteKey(Key);
	WriteRaw(FString::Printf(TEXT("%lld"), Value));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, double Value, int32 Decimals)
{
	WriteKey(Key);
	WriteRaw(FormatNumber(Value, Decimals));
	return *this;
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, const FVector& Value)
{
	BeginArray(Key);
	Write(nullptr, static_cast<double>(Value.X), 2);
	Write(nullptr, static_cast<double>(Value.Y), 2);
	Write(nullptr, static_cast<double>(Value.Z), 2);
	return EndArray();
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, const FRotator& Value)
{
	BeginArray(Key);
	Write(nullptr, static_cast<double>(Value.Pitch), 2);
	Write(nullptr, static_cast<double>(Value.Yaw), 2);
	Write(nullptr, static_cast<double>(Value.Roll), 2);
	return EndArray();
}

FSentryGameStateJsonWriter& FSentryGameStateJsonWriter::Write(const TCHAR* Key, const FSentryVariant& Value)
{
	switch (Value.GetType())
	{
	case ESentryVariantType::Integer:
		return Write(Key, Value.GetValue<int32>());
	case ESentryVariantType::Float:
		return Write(Key, static_cast<double>(Value.GetValue<float>()), 4);
	case ESentryVariantType::Bool:
		return Write(Key, Value.GetValue<bool>());
	case ESentryVariantType::String:
		return Write(Key, Value.GetValue<FString>());
	case ESentryVariantType::Array:
	{
		BeginArray(Key);
		for (const FSentryVariant& Element : Value.GetValue<TArray<FSentryVariant>>())
		{
			Write(nullptr, Element);
		}
		return EndArray();
	}
	case ESentryVariantType::Map:
	{
		BeginObject(Key);
		for (const TPair<FString, FSentryVariant>& Pair : Value.GetValue<TMap<FString, FSentryVariant>>())
		{
			Write(*Pair.Key, Pair.Value);
		}
		return EndObject();
	}
	default:
		WriteKey(Key);
		WriteRaw(TEXT("null"));
		return *this;
	}
}

FString FSentryGameStateJsonWriter::EscapeString(const FString& Value)
{
	FString Escaped;
	Escaped.Reserve(Value.Len());

	for (const TCHAR Char : Value)
	{
		switch (Char)
		{
		case TEXT('"'):
			Escaped.Append(TEXT("\\\""));
			break;
		case TEXT('\\'):
			Escaped.Append(TEXT("\\\\"));
			break;
		case TEXT('\n'):
			Escaped.Append(TEXT("\\n"));
			break;
		case TEXT('\r'):
			Escaped.Append(TEXT("\\r"));
			break;
		case TEXT('\t'):
			Escaped.Append(TEXT("\\t"));
			break;
		default:
			if (Char < 0x20)
			{
				Escaped.Append(FString::Printf(TEXT("\\u%04x"), static_cast<int32>(Char)));
			}
			else
			{
				Escaped.AppendChar(Char);
			}
		}
	}

	return Escaped;
}

FString FSentryGameStateJsonWriter::FormatNumber(double Value, int32 Decimals)
{
	if (!FMath::IsFinite(Value))
	{
		return TEXT("null");
	}

	FString Formatted;
	switch (FMath::Clamp(Decimals, 0, 4))
	{
	case 0:
		Formatted = FString::Printf(TEXT("%.0f"), Value);
		break;
	case 1:
		Formatted = FString::Printf(TEXT("%.1f"), Value);
		break;
	case 2:
		Formatted = FString::Printf(TEXT("%.2f"), Value);
		break;
	case 3:
		Formatted = FString::Printf(TEXT("%.3f"), Value);
		break;
	default:
		Formatted = FString::Printf(TEXT("%.4f"), Value);
		break;
	}

	if (Formatted.Contains(TEXT(".")))
	{
		int32 End = Formatted.Len();
		while (End > 0 && Formatted[End - 1] == TEXT('0'))
		{
			--End;
		}
		if (End > 0 && Formatted[End - 1] == TEXT('.'))
		{
			--End;
		}
		Formatted.LeftInline(End);
	}

	if (Formatted == TEXT("-0"))
	{
		Formatted = TEXT("0");
	}

	return Formatted;
}

void FSentryGameStateJsonWriter::WriteKey(const TCHAR* Key)
{
	if (HasElements.Num() > 0)
	{
		if (HasElements.Last())
		{
			Output.AppendChar(TEXT(','));
		}
		HasElements.Last() = true;
	}

	if (Key)
	{
		Output.AppendChar(TEXT('"'));
		Output.Append(EscapeString(Key));
		Output.Append(TEXT("\":"));
	}
}

void FSentryGameStateJsonWriter::WriteRaw(const FString& Value)
{
	Output.Append(Value);
}
