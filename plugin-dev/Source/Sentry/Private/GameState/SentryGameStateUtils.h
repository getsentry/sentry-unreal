// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SentryGameStateUtils
{
public:
	/** Converts a record to a UTF-8 line terminated with a newline. */
	static TArray<uint8> ToUtf8Line(const FString& Line);
};
