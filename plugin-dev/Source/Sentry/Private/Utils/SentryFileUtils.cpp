// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryFileUtils.h"
#include "SentryDefines.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

struct FSentrySortFileByDatePredicate
{
	bool operator()(const FString& A, const FString& B) const
	{
		const FDateTime TimestampA = IFileManager::Get().GetTimeStamp(*A);
		const FDateTime TimestampB = IFileManager::Get().GetTimeStamp(*B);
		return TimestampB < TimestampA;
	}
};

FString SentryFileUtils::GetGameLogName()
{
	return FPaths::GetCleanFilename(FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
}

FString SentryFileUtils::GetGameLogPath()
{
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
}

FString SentryFileUtils::GetGameLogBackupPath()
{
	TArray<FString> GameLogBackupFiles;
	IFileManager::Get().FindFiles(GameLogBackupFiles, *FString::Printf(TEXT("%s*-backup-*.*"), *FPaths::ProjectLogDir()), true, false);

	if (GameLogBackupFiles.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("There are no game log backups available."));
		return FString("");
	}

	for (int i = 0; i < GameLogBackupFiles.Num(); ++i)
	{
		GameLogBackupFiles[i] = FPaths::ProjectLogDir() / GameLogBackupFiles[i];
	}

	GameLogBackupFiles.Sort(FSentrySortFileByDatePredicate());

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*GameLogBackupFiles[0]);
}

FString SentryFileUtils::GetGpuDumpPath()
{
	TArray<FString> GpuDumpFiles;
	IFileManager::Get().FindFiles(GpuDumpFiles, *FString::Printf(TEXT("%s*.nv-gpudmp"), *FPaths::ProjectLogDir()), true, false);

	if (GpuDumpFiles.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("There is no GPU dump file available."));
		return FString("");
	}

	// By default, engine cleans up GPU dumps from the previous runs however this doesn't seem to be the case
	// if https://github.com/EpicGames/UnrealEngine/pull/12648 patch is applied so we just return the newest one

	for (int i = 0; i < GpuDumpFiles.Num(); ++i)
	{
		GpuDumpFiles[i] = FPaths::ProjectLogDir() / GpuDumpFiles[i];
	}

	GpuDumpFiles.Sort(FSentrySortFileByDatePredicate());

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*GpuDumpFiles[0]);
}
