// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

class FEvent;
class FRunnableThread;

/**
 * Writes game state snapshots to a file on a dedicated thread.
 *
 * Only the most recent snapshot is kept if the thread falls behind. A dedicated thread is used
 * since shared worker pools may be busy for seconds (e.g. with asset loading), leaving the file stale.
 */
class FSentryGameStateFileWriter final : public FRunnable
{
public:
	explicit FSentryGameStateFileWriter(const FString& InFilePath);
	virtual ~FSentryGameStateFileWriter() override;

	/** Queues the snapshot to be written, replacing any snapshot that is not written yet. */
	void Submit(TArray<uint8>&& Snapshot);

	/** Drops the queued snapshot, waits for the write in progress and deletes the file. */
	void DiscardAndDeleteFile();

	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	void WriteSnapshot(const TArray<uint8>& Snapshot);
	void DeleteFiles();

	FString FilePath;
	FString TempFilePath;

	FCriticalSection QueueLock;
	TArray<uint8> QueuedSnapshot;
	bool bHasQueuedSnapshot = false;

	FCriticalSection WriteLock;

	FEvent* WakeUpEvent = nullptr;
	FThreadSafeBool bStopping;
	FRunnableThread* Thread = nullptr;
};
