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
	static TArray<FString> GetGpuShaderDebugInfoPaths();
	static FString GetScreenshotPath();
	static FString GetLatestScreenshot();
	static FString GetGameStatePath();
	static FString GetGameStateBackupPath();

	/** Moves the previous run's game state file to the backup location so it can be attached to that run's crash. */
	static void BackupGameState();
};
