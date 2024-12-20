// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryFileUtils.h"
#include "SentryDefines.h"

#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"
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

FString SentryFileUtils::GetGameLogPath()
{
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
}

FString SentryFileUtils::GetGameLogBackupPath()
{
	TArray<FString> GameLogBackupFiles;
	IFileManager::Get().FindFiles(GameLogBackupFiles, *FString::Printf(TEXT("%s*-backup-*.*"), *FPaths::ProjectLogDir()), true, false);

	if(GameLogBackupFiles.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("There are no game log backups available."));
		return FString("");
	}

	for (int i = 0; i < GameLogBackupFiles.Num(); ++i)
	{
		FString GameLogFullPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectLogDir() / GameLogBackupFiles[i]));
		GameLogBackupFiles[i] = GameLogFullPath;
	}

	GameLogBackupFiles.Sort(FSentrySortFileByDatePredicate());

	return GameLogBackupFiles[0];
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

	if (GpuDumpFiles.Num() > 1)
	{
		// By default, engine should handle clean up of GPU dumps  from the previous runs
		UE_LOG(LogSentrySdk, Log, TEXT("There are multiple GPU dump files, can't determine reliably which one to pick."));
		return FString("");
	}

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectLogDir() / GpuDumpFiles[0]));
}
