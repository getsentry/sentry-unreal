// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SentryFileUtils
{
public:
	static FString GetGameLogPath();
	static FString GetGameLogBackupPath();
	static FString GetGpuDumpPath();
};