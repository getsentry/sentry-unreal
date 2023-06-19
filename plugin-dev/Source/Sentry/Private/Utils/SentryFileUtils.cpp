// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryFileUtils.h"
#include "SentryDefines.h"

#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"

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

	if(GameLogBackupFiles.IsEmpty())
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