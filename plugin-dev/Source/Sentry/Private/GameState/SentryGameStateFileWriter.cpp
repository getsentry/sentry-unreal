// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateFileWriter.h"

#include "SentryDefines.h"

#include "HAL/Event.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"

FSentryGameStateFileWriter::FSentryGameStateFileWriter(const FString& InFilePath)
	: FilePath(InFilePath)
	, TempFilePath(InFilePath + TEXT(".tmp"))
{
	DeleteFiles();

	WakeUpEvent = FPlatformProcess::GetSynchEventFromPool(false);
	Thread = FRunnableThread::Create(this, TEXT("SentryGameStateWriter"), 0, TPri_BelowNormal);
}

FSentryGameStateFileWriter::~FSentryGameStateFileWriter()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}

	FPlatformProcess::ReturnSynchEventToPool(WakeUpEvent);
	WakeUpEvent = nullptr;

	DeleteFiles();
}

void FSentryGameStateFileWriter::Submit(TArray<uint8>&& Snapshot)
{
	{
		FScopeLock Lock(&QueueLock);
		QueuedSnapshot = MoveTemp(Snapshot);
		bHasQueuedSnapshot = true;
	}

	WakeUpEvent->Trigger();
}

void FSentryGameStateFileWriter::DiscardAndDeleteFile()
{
	{
		FScopeLock Lock(&QueueLock);
		QueuedSnapshot.Empty();
		bHasQueuedSnapshot = false;
	}

	FScopeLock Lock(&WriteLock);
	DeleteFiles();
}

uint32 FSentryGameStateFileWriter::Run()
{
	while (!bStopping)
	{
		WakeUpEvent->Wait();

		FScopeLock WriteScope(&WriteLock);

		TArray<uint8> Snapshot;
		{
			FScopeLock QueueScope(&QueueLock);
			if (!bHasQueuedSnapshot)
			{
				continue;
			}

			Snapshot = MoveTemp(QueuedSnapshot);
			bHasQueuedSnapshot = false;
		}

		WriteSnapshot(Snapshot);
	}

	return 0;
}

void FSentryGameStateFileWriter::Stop()
{
	bStopping = true;
	WakeUpEvent->Trigger();
}

void FSentryGameStateFileWriter::WriteSnapshot(const TArray<uint8>& Snapshot)
{
	TUniquePtr<FArchive> Writer(IFileManager::Get().CreateFileWriter(*TempFilePath, FILEWRITE_EvenIfReadOnly));
	if (!Writer)
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to open %s for write"), *TempFilePath);
		return;
	}

	Writer->Serialize(const_cast<uint8*>(Snapshot.GetData()), Snapshot.Num());

	if (!Writer->Close())
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to write %s"), *TempFilePath);
		return;
	}

	if (!IFileManager::Get().Move(*FilePath, *TempFilePath, true, true, false, true))
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Game state capture: failed to replace %s"), *FilePath);
	}
}

void FSentryGameStateFileWriter::DeleteFiles()
{
	IFileManager::Get().Delete(*FilePath, false, true, true);
	IFileManager::Get().Delete(*TempFilePath, false, true, true);
}
