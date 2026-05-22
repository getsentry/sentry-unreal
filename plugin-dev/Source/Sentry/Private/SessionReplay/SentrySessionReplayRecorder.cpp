// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentrySessionReplayRecorder.h"

#if USE_SENTRY_SESSION_REPLAY

#include "SentryBackBufferCapture.h"
#include "SentryDefines.h"
#include "SentrySettings.h"
#include "SentryVideoEncoder.h"

#include "HAL/Event.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"

FSentrySessionReplayRecorder::FSentrySessionReplayRecorder() = default;

FSentrySessionReplayRecorder::~FSentrySessionReplayRecorder()
{
	Shutdown();
}

bool FSentrySessionReplayRecorder::Initialize(const USentrySettings* Settings, const FString& ReplayPath)
{
	check(IsInGameThread());

	if (!Settings || !Settings->AttachSessionReplay)
	{
		return false;
	}

	WindowSeconds = Settings->SessionReplayWindowSeconds;
	FragmentSeconds = Settings->SessionReplayFragmentSeconds;
	RotationIntervalSeconds = Settings->SessionReplayRotationIntervalSeconds;
	FragmentRingCapacity = FMath::Max(2, FMath::CeilToInt(WindowSeconds / FMath::Max(0.1f, FragmentSeconds)));
	FragmentRing.Empty(FragmentRingCapacity);

	// Use the caller-supplied per-session path (typically
	// `<.sentry-native>/replays/replay-<guid>.mp4`). The temp file used by
	// atomic-rename rotation is sibling to the target.
	AttachmentPath = ReplayPath;
	TempPath = ReplayPath + TEXT(".tmp");
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AttachmentPath), /*Tree*/ true);
	bSnapshotOnDisk.AtomicSet(false);

	// The encoder picks Width/Height from the first submitted frame (native
	// backbuffer dimensions). A configurable target resolution would require
	// a scaling draw pass — out of scope for v1.
	Encoder = MakeUnique<FSentryVideoEncoder>(
		*this,
		static_cast<uint32>(Settings->SessionReplayFramerate),
		Settings->SessionReplayBitrateKbps,
		Settings->SessionReplayFragmentSeconds);
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

	// Spawn rotation thread last, so it doesn't fire before the queues are ready.
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

	// Stop the rotation thread first so it doesn't race against teardown.
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

void FSentrySessionReplayRecorder::OnInitSegmentReady(TArray<uint8>&& NewInitSegment)
{
	FScopeLock Lock(&RingLock);
	InitSegment = MoveTemp(NewInitSegment);
}

void FSentrySessionReplayRecorder::OnFragmentReady(TArray<uint8>&& Fragment)
{
	FScopeLock Lock(&RingLock);
	if (FragmentRing.Num() >= FragmentRingCapacity)
	{
		FragmentRing.PopFront();
	}
	FragmentRing.Add(MoveTemp(Fragment));
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
	// Read the tfdt's base_media_decode_time from a serialised fragment. The
	// field lives at a fixed offset inside our fragment layout:
	//   moof box header (8) + mfhd (16) + traf box header (8)
	//   + tfhd (16) + tfdt size+type+ver+flags (12) = 60.
	// And it's a big-endian 64-bit value (we always emit tfdt v1).
	constexpr int32 TfdtFieldOffset = 60;
	auto ReadTfdt = [&](const TArray<uint8>& Bytes) -> uint64
	{
		if (Bytes.Num() < TfdtFieldOffset + 8)
			return 0;
		uint64 V = 0;
		for (int32 b = 0; b < 8; ++b)
		{
			V = (V << 8) | static_cast<uint64>(Bytes[TfdtFieldOffset + b]);
		}
		return V;
	};
	auto WriteTfdtAt = [&](TArray<uint8>& Bytes, int32 Offset, uint64 V)
	{
		if (Bytes.Num() < Offset + 8)
			return;
		for (int32 b = 0; b < 8; ++b)
		{
			Bytes[Offset + (7 - b)] = static_cast<uint8>(V & 0xFF);
			V >>= 8;
		}
	};

	TArray<uint8> Snapshot;
	{
		FScopeLock Lock(&RingLock);
		if (InitSegment.Num() == 0 || FragmentRing.Num() == 0)
		{
			return; // no encoded data yet
		}

		// Reserve roughly:
		int64 Reserve = InitSegment.Num();
		for (int32 i = 0; i < FragmentRing.Num(); ++i)
		{
			Reserve += FragmentRing[i].Num();
		}
		Snapshot.Reserve(static_cast<int32>(Reserve));
		Snapshot.Append(InitSegment);

		// Rebase tfdt so the surviving fragments form a clip starting at t=0.
		// Without this, evicted fragments leave the kept ones with absolute
		// session-clock tfdt values, and players that compute duration from
		// "last sample end time" overstate the clip length.
		const uint64 FirstTfdt = ReadTfdt(FragmentRing[0]);
		for (int32 i = 0; i < FragmentRing.Num(); ++i)
		{
			const int32 FragStartInSnapshot = Snapshot.Num();
			Snapshot.Append(FragmentRing[i]);

			const uint64 OrigTfdt = ReadTfdt(FragmentRing[i]);
			const uint64 NewTfdt = (OrigTfdt >= FirstTfdt) ? (OrigTfdt - FirstTfdt) : 0;
			WriteTfdtAt(Snapshot, FragStartInSnapshot + TfdtFieldOffset, NewTfdt);
		}
	}

	if (WriteSnapshotAtomically(Snapshot))
	{
		bSnapshotOnDisk.AtomicSet(true);
	}
}

bool FSentrySessionReplayRecorder::WriteSnapshotAtomically(const TArray<uint8>& Bytes)
{
	// 1) Write bytes into the temp file (truncate any prior remnants).
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
	}

	// 2) Replace the previous snapshot. IFileManager::Move with bReplace=true
	//    deletes the destination then renames — not strictly atomic, but the
	//    rename itself is atomic on both Windows (MoveFileW) and POSIX
	//    (rename(2)). The brief window between delete and rename where the
	//    file is absent is harmless: crashpad logs "attachment couldn't be
	//    opened, skipping" and uploads the report without the video — the
	//    same fallback we already rely on for early-session crashes that
	//    fire before the first snapshot is produced.
	if (!IFileManager::Get().Move(*AttachmentPath, *TempPath, /*bReplace*/ true, /*EvenIfReadOnly*/ true))
	{
		static bool bLoggedOnce = false;
		if (!bLoggedOnce)
		{
			UE_LOG(LogSentrySdk, Warning,
				TEXT("Session replay: failed to replace %s — another process may be holding it open ")
					TEXT("(e.g. a video preview window). Rotation will keep retrying. In a normal crash scenario this doesn't happen."),
				*AttachmentPath);
			bLoggedOnce = true;
		}
		return false;
	}
	return true;
}

#endif // USE_SENTRY_SESSION_REPLAY
