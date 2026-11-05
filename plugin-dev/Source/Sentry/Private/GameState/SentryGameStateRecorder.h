// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Keeps a rolling window of game state records (JSONL lines) in memory.
 *
 * Records older than the window or exceeding the size limit are dropped, oldest first.
 * Not thread-safe - must be used from a single thread.
 */
class FSentryGameStateRecorder
{
public:
	FSentryGameStateRecorder(double InWindowSeconds, int32 InMaxSizeBytes);

	/** Sets the line written at the top of every snapshot. */
	void SetHeader(const FString& InHeader);

	/** Adds a record captured at the given time (in seconds) and drops records that fell out of the window. */
	void AddRecord(double Time, const FString& Record);

	/** Drops all records. Keeps the header. */
	void Reset();

	int32 GetNumRecords() const { return Records.Num(); }
	int32 GetSizeBytes() const { return SizeBytes; }

	/** Builds a UTF-8 JSONL snapshot: the header line followed by all records in the window. */
	TArray<uint8> BuildSnapshot() const;

private:
	struct FRecord
	{
		double Time;
		TArray<uint8> Data;
	};

	void Trim(double LatestTime);

	double WindowSeconds;
	int32 MaxSizeBytes;

	TArray<uint8> Header;
	TArray<FRecord> Records;
	int32 SizeBytes = 0;
};
