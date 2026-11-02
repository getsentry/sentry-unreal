// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GameState/SentryGameStateRecorder.h"

#include "GameState/SentryGameStateUtils.h"

FSentryGameStateRecorder::FSentryGameStateRecorder(double InWindowSeconds, int32 InMaxSizeBytes)
	: WindowSeconds(InWindowSeconds)
	, MaxSizeBytes(InMaxSizeBytes)
{
}

void FSentryGameStateRecorder::SetHeader(const FString& InHeader)
{
	Header = SentryGameStateUtils::ToUtf8Line(InHeader);
}

void FSentryGameStateRecorder::AddRecord(double Time, const FString& Record)
{
	FRecord& Added = Records.AddDefaulted_GetRef();
	Added.Time = Time;
	Added.Data = SentryGameStateUtils::ToUtf8Line(Record);

	SizeBytes += Added.Data.Num();

	Trim(Time);
}

void FSentryGameStateRecorder::Reset()
{
	Records.Reset();
	SizeBytes = 0;
}

TArray<uint8> FSentryGameStateRecorder::BuildSnapshot() const
{
	TArray<uint8> Snapshot;
	Snapshot.Reserve(Header.Num() + SizeBytes);
	Snapshot.Append(Header);

	for (const FRecord& Record : Records)
	{
		Snapshot.Append(Record.Data);
	}

	return Snapshot;
}

void FSentryGameStateRecorder::Trim(double LatestTime)
{
	const double OldestAllowedTime = LatestTime - WindowSeconds;

	int32 NumToRemove = 0;
	int32 RemainingSize = SizeBytes;

	while (NumToRemove < Records.Num() - 1 &&
		   (Records[NumToRemove].Time < OldestAllowedTime || RemainingSize > MaxSizeBytes))
	{
		RemainingSize -= Records[NumToRemove].Data.Num();
		++NumToRemove;
	}

	if (NumToRemove > 0)
	{
		Records.RemoveAt(0, NumToRemove);
		SizeBytes = RemainingSize;
	}
}
