// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateUtils.h"

#include "SentryDefines.h"

#include "Containers/StringConv.h"
#include "HAL/FileManager.h"

TArray<uint8> SentryGameStateUtils::ToUtf8Line(const FString& Line)
{
	FTCHARToUTF8 Converted(*Line);

	TArray<uint8> Data;
	Data.Reserve(Converted.Length() + 1);
	Data.Append(reinterpret_cast<const uint8*>(Converted.Get()), Converted.Length());
	Data.Add('\n');
	return Data;
}

const TCHAR* SentryGameStateUtils::NetModeToString(ENetMode NetMode)
{
	switch (NetMode)
	{
	case NM_DedicatedServer:
		return TEXT("dedicated_server");
	case NM_ListenServer:
		return TEXT("listen_server");
	case NM_Client:
		return TEXT("client");
	default:
		return TEXT("standalone");
	}
}

void SentryGameStateUtils::WriteSnapshotFile(const FString& Path, const TArray<uint8>& Data)
{
	const FString TempPath = Path + TEXT(".tmp");

	TUniquePtr<FArchive> Writer(IFileManager::Get().CreateFileWriter(*TempPath, FILEWRITE_EvenIfReadOnly));
	if (!Writer)
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to open %s for write"), *TempPath);
		return;
	}

	Writer->Serialize(const_cast<uint8*>(Data.GetData()), Data.Num());

	if (!Writer->Close())
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to write %s"), *TempPath);
		return;
	}

	if (!IFileManager::Get().Move(*Path, *TempPath, true, true, false, true))
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to replace %s"), *Path);
	}
}
