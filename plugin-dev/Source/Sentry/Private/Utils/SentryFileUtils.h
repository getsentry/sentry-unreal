// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SentryFileUtils
{
public:
	static FString GetGameLogName();
	static FString GetGameLogPath();
	static FString GetGameLogBackupPath();
	static FString GetGpuDumpPath();
};