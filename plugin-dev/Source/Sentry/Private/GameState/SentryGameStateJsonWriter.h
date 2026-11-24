// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FSentryVariant;

/**
 * Minimal single-line JSON writer for game state records.
 *
 * Unlike TJsonWriter it writes floating-point values with fixed precision, which keeps
 * per-frame records compact.
 */
class FSentryGameStateJsonWriter
{
public:
	/** Begins an object. Key must be null when writing an array element or the root object. */
	FSentryGameStateJsonWriter& BeginObject(const TCHAR* Key = nullptr);
	FSentryGameStateJsonWriter& EndObject();

	/** Begins an array. Key must be null when writing an array element. */
	FSentryGameStateJsonWriter& BeginArray(const TCHAR* Key = nullptr);
	FSentryGameStateJsonWriter& EndArray();

	FSentryGameStateJsonWriter& Write(const TCHAR* Key, const FString& Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, const TCHAR* Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, bool Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, int32 Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, int64 Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, double Value, int32 Decimals = 3);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, const FVector& Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, const FRotator& Value);
	FSentryGameStateJsonWriter& Write(const TCHAR* Key, const FSentryVariant& Value);

	const FString& GetOutput() const { return Output; }

	static FString EscapeString(const FString& Value);
	static FString FormatNumber(double Value, int32 Decimals);

private:
	void WriteKey(const TCHAR* Key);
	void WriteRaw(const FString& Value);

	FString Output;
	TArray<bool> HasElements;
};
