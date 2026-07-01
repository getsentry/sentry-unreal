// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentrySessionReplayRecorder.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "SentryBackBufferCapture.h"
#include "SentryDefines.h"
#include "SentrySettings.h"
#include "SentryVideoEncoder.h"

#include "HAL/Event.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "JsonObjectConverter.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"

FSentrySessionReplayRecorder::FSentrySessionReplayRecorder() = default;

FSentrySessionReplayRecorder::~FSentrySessionReplayRecorder()
{
	Shutdown();
}

bool FSentrySessionReplayRecorder::Initialize(const USentrySettings* Settings, const FString& InReplayId, const FString& ReplayPath)
{
	check(IsInGameThread());

	if (!Settings || !Settings->AttachSessionReplay)
	{
		return false;
	}

	ReplayId = InReplayId;

	if (!FApp::CanEverRender())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Session replay disabled: no rendering RHI available (commandlet, dedicated server, or -NullRHI)"));
		return false;
	}

	WindowSeconds = Settings->SessionReplayDurationMs / 1000.0f;
	FragmentSeconds = Settings->SessionReplayOptions.FragmentSeconds;
	RotationIntervalSeconds = Settings->SessionReplayOptions.RotationIntervalSeconds;

	FragmentRingCapacity = FMath::Max(2, FMath::CeilToInt(WindowSeconds / FMath::Max(0.1f, FragmentSeconds)));
	FragmentRing.Empty(FragmentRingCapacity);

	AttachmentPath = ReplayPath;
	TempPath = ReplayPath + TEXT(".tmp");
	MetadataPath = FPaths::ChangeExtension(ReplayPath, TEXT("json"));
	MetadataTempPath = MetadataPath + TEXT(".tmp");

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AttachmentPath), true);

	bSnapshotOnDisk.AtomicSet(false);

	Encoder = MakeUnique<FSentryVideoEncoder>(*this, static_cast<uint32>(Settings->SessionReplayOptions.Framerate), Settings->SessionReplayOptions.BitrateKbps, Settings->SessionReplayOptions.FragmentSeconds);
	if (!Encoder->StartEncoder())
	{
		Encoder.Reset();

		return false;
	}

	Capture = MakeUnique<FSentryBackBufferCapture>(*Encoder);
	if (!Capture->Start())
	{
		Capture.Reset();

		Encoder->StopEncoder();
		Encoder.Reset();

		return false;
	}

	bStopRequested.AtomicSet(false);

	RotationWake = FPlatformProcess::GetSynchEventFromPool(false);
	RotationThread = FRunnableThread::Create(this, TEXT("SentrySessionReplayRotation"), 0, TPri_BelowNormal);
	if (!RotationThread)
	{
		Capture->Stop();
		Capture.Reset();

		Encoder->StopEncoder();
		Encoder.Reset();

		FPlatformProcess::ReturnSynchEventToPool(RotationWake);
		RotationWake = nullptr;

		return false;
	}

	bEnabled = true;

	UE_LOG(LogSentrySdk, Log, TEXT("Session replay enabled, attachment path: %s"), *AttachmentPath);

	return true;
}

void FSentrySessionReplayRecorder::Shutdown()
{
	check(IsInGameThread());

	if (!bEnabled)
	{
		return;
	}
	bEnabled = false;

	// Stop the rotation thread first so it doesn't race against teardown
	bStopRequested.AtomicSet(true);
	if (RotationWake)
	{
		RotationWake->Trigger();
	}
	if (RotationThread)
	{
		RotationThread->WaitForCompletion();
		delete RotationThread;
		RotationThread = nullptr;
	}
	if (RotationWake)
	{
		FPlatformProcess::ReturnSynchEventToPool(RotationWake);
		RotationWake = nullptr;
	}

	if (Capture)
	{
		Capture->Stop();
		Capture.Reset();
	}
	if (Encoder)
	{
		Encoder->StopEncoder();
		Encoder.Reset();
	}

	{
		FScopeLock Lock(&RingLock);
		InitSegment.Empty();
		FragmentRing.Reset();
	}
}

FSentryReplayInfo FSentrySessionReplayRecorder::BuildReplayInfo() const
{
	const FDateTime NowUtc = FDateTime::UtcNow();

	FSentryReplayInfo Info;
	Info.ReplayId = ReplayId;
	Info.Width = Encoder ? static_cast<int32>(Encoder->GetWidth()) : 0;
	Info.Height = Encoder ? static_cast<int32>(Encoder->GetHeight()) : 0;
	Info.FrameRate = Encoder ? static_cast<int32>(Encoder->GetFramerate()) : 0;
	Info.DurationMs = LatestDurationMs;
	Info.FrameCount = LatestFrameCount;
	Info.SizeBytes = IFileManager::Get().FileSize(*AttachmentPath);
	Info.EndTimestampSec = static_cast<double>(NowUtc.ToUnixTimestamp()) + NowUtc.GetMillisecond() / 1000.0;
	Info.StartTimestampSec = Info.EndTimestampSec - static_cast<double>(Info.DurationMs) / 1000.0;

	return Info;
}

void FSentrySessionReplayRecorder::OnInitSegmentReady(TArray<uint8>&& NewInitSegment)
{
	FScopeLock Lock(&RingLock);
	InitSegment = MoveTemp(NewInitSegment);

	if (!FragmentRing.IsEmpty())
	{
		FragmentRing.Reset();
	}
}

void FSentrySessionReplayRecorder::OnFragmentReady(TArray<uint8>&& Fragment, uint32 FrameCount, uint64 DurationTicks)
{
	FScopeLock Lock(&RingLock);
	if (FragmentRing.Num() >= FragmentRingCapacity)
	{
		FragmentRing.PopFront();
	}

	FFragment Entry;
	Entry.Bytes = MoveTemp(Fragment);
	Entry.FrameCount = FrameCount;
	Entry.DurationTicks = DurationTicks;

	FragmentRing.Add(MoveTemp(Entry));
}

bool FSentrySessionReplayRecorder::Init()
{
	return true;
}

uint32 FSentrySessionReplayRecorder::Run()
{
	const uint32 SleepMs = FMath::Max(50, FMath::RoundToInt(RotationIntervalSeconds * 1000.0f));
	while (!bStopRequested)
	{
		if (RotationWake)
		{
			RotationWake->Wait(SleepMs);
		}
		if (bStopRequested)
		{
			break;
		}
		DoRotation();
	}
	return 0;
}

void FSentrySessionReplayRecorder::Stop()
{
	bStopRequested.AtomicSet(true);
	if (RotationWake)
	{
		RotationWake->Trigger();
	}
}

void FSentrySessionReplayRecorder::Exit()
{
}

void FSentrySessionReplayRecorder::DoRotation()
{
	// The tfdt's base_media_decode_time lives at a fixed byte offset inside
	// each serialized fragment, given the layout produced by FSentryFMP4Writer:
	// moof header (8) + mfhd (16) + traf header (8) + tfhd (16) + tfdt size/type/ver/flags (12) = 60
	// followed by an 8-byte big-endian uint64 value (tfdt v1).
	constexpr int32 TfdtFieldOffset = 60;

	TArray<uint8> Snapshot;
	int64 TotalFrames = 0;
	uint64 TotalTicks = 0;
	{
		FScopeLock Lock(&RingLock);
		if (InitSegment.Num() == 0 || FragmentRing.Num() == 0)
		{
			return;
		}

		int64 Reserve = InitSegment.Num();
		for (int32 i = 0; i < FragmentRing.Num(); ++i)
		{
			Reserve += FragmentRing[i].Bytes.Num();
		}
		Snapshot.Reserve(static_cast<int32>(Reserve));
		Snapshot.Append(InitSegment);

		// Rebase tfdt so the surviving fragments form a clip starting at t=0.
		// Without this, evicted fragments leave the kept ones with absolute
		// session-clock tfdt values, and players that compute duration from
		// "last sample end time" overstate the clip length
		const uint64 FirstTfdt = FSentryFMP4Writer::ReadU64BE(FragmentRing[0].Bytes, TfdtFieldOffset);
		for (int32 i = 0; i < FragmentRing.Num(); ++i)
		{
			const FFragment& Fragment = FragmentRing[i];
			const int32 FragStartInSnapshot = Snapshot.Num();
			Snapshot.Append(Fragment.Bytes);

			const uint64 OrigTfdt = FSentryFMP4Writer::ReadU64BE(Fragment.Bytes, TfdtFieldOffset);
			const uint64 NewTfdt = (OrigTfdt >= FirstTfdt) ? (OrigTfdt - FirstTfdt) : 0;
			FSentryFMP4Writer::PatchU64(Snapshot, FragStartInSnapshot + TfdtFieldOffset, NewTfdt);

			TotalFrames += Fragment.FrameCount;
			TotalTicks += Fragment.DurationTicks;
		}
	}

	LatestFrameCount = static_cast<int32>(TotalFrames);
	LatestDurationMs = static_cast<int64>((TotalTicks * 1000) / FSentryFMP4Writer::TrackTimescale);

	if (WriteSnapshot(Snapshot))
	{
		bSnapshotOnDisk.AtomicSet(true);

		WriteReplayMetadata();
	}
}

bool FSentrySessionReplayRecorder::WriteSnapshot(const TArray<uint8>& Bytes)
{
	TUniquePtr<FArchive> Out(IFileManager::Get().CreateFileWriter(*TempPath, FILEWRITE_EvenIfReadOnly));
	if (!Out)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to open %s for write"), *TempPath);
		return false;
	}
	Out->Serialize(const_cast<uint8*>(Bytes.GetData()), Bytes.Num());
	Out->Flush();
	if (!Out->Close())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to close temp file"));
		return false;
	}

	if (!IFileManager::Get().Move(*AttachmentPath, *TempPath, true, true))
	{
		static bool bLoggedOnce = false;
		if (!bLoggedOnce)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to replace %s"), *AttachmentPath);
			bLoggedOnce = true;
		}
		return false;
	}
	return true;
}

void FSentrySessionReplayRecorder::WriteReplayMetadata()
{
	FString Json;
	if (!FJsonObjectConverter::UStructToJsonObjectString(BuildReplayInfo(), Json, 0, 0, 0, nullptr, false))
	{
		return;
	}

	if (!FFileHelper::SaveStringToFile(Json, *MetadataTempPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to write metadata temp file at %s"), *MetadataTempPath);
		return;
	}

	if (!IFileManager::Get().Move(*MetadataPath, *MetadataTempPath, true, true))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Session replay: failed to replace metadata at %s"), *MetadataPath);
	}
}

#endif // USE_SENTRY_SESSION_REPLAY
