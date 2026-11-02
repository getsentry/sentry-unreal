// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateUtils.h"

#include "Containers/StringConv.h"

TArray<uint8> SentryGameStateUtils::ToUtf8Line(const FString& Line)
{
	FTCHARToUTF8 Converted(*Line);

	TArray<uint8> Data;
	Data.Reserve(Converted.Length() + 1);
	Data.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	Data.Add('\n');
	return Data;
}
