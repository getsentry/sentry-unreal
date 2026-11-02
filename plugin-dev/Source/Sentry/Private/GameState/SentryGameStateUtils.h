// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"

class SentryGameStateUtils
{
public:
	/** Converts a record to a UTF-8 line terminated with a newline. */
	static TArray<uint8> ToUtf8Line(const FString& Line);

	static const TCHAR* NetModeToString(ENetMode NetMode);

	/** Writes the data to a temporary file and moves it over the given path. */
	static void WriteSnapshotFile(const FString& Path, const TArray<uint8>& Data);
};
