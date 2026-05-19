// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryCrashVideoSubsystem.h"

#if USE_SENTRY_CRASH_VIDEO

#include "SentryBackBufferCapture.h"
#include "SentryDefines.h"
#include "SentryFmp4Writer.h"
#include "SentrySettings.h"
#include "SentryVideoEncoder.h"

#include "HAL/Event.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/WindowsHWrapper.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

namespace
{
	void GetResolutionExtents(ESentryCrashVideoResolution Resolution, uint32& OutWidth, uint32& OutHeight)
	{
		switch (Resolution)
		{
		case ESentryCrashVideoResolution::SD480:
			OutWidth = 854; OutHeight = 480; break;
		case ESentryCrashVideoResolution::HD1080:
			OutWidth = 1920; OutHeight = 1080; break;
		case ESentryCrashVideoResolution::MatchBackBuffer:
			OutWidth = 0; OutHeight = 0; break;
		case ESentryCrashVideoResolution::HD720:
		default:
			OutWidth = 1280; OutHeight = 720; break;
		}
	}
}

FSentryCrashVideoSubsystem::FSentryCrashVideoSubsystem() = default;

FSentryCrashVideoSubsystem::~FSentryCrashVideoSubsystem()
{
	Shutdown();
}

bool FSentryCrashVideoSubsystem::Initialize(const USentrySettings* Settings)
{
	check(IsInGameThread());

	if (!Settings || !Settings->EnableCrashVideo)
	{
		return false;
	}
	if (Settings->UseNativeBackend)
	{
		UE_LOG(LogSentrySdk, Warning,
			TEXT("Crash video requires the Crashpad backend; UseNativeBackend is set, feature disabled."));
		return false;
	}

	WindowSeconds = Settings->CrashVideoWindowSeconds;
	FragmentSeconds = Settings->CrashVideoFragmentSeconds;
	RotationIntervalSeconds = Settings->CrashVideoRotationIntervalSeconds;
	FragmentRingCapacity = FMath::Max(2, FMath::CeilToInt(WindowSeconds / FMath::Max(0.1f, FragmentSeconds)));
	FragmentRing.Empty(FragmentRingCapacity);

	// Build the attachment path. Both .mp4 and .tmp live next to the Sentry database
	// inside ProjectSavedDir/Sentry/. Crashpad reads the .mp4 path at crash time.
	const FString SentryDir = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectSavedDir() / TEXT("Sentry"));
	IFileManager::Get().MakeDirectory(*SentryDir, /*Tree*/ true);
	AttachmentPath = SentryDir / TEXT("crash_video.mp4");
	TempPath = SentryDir / TEXT("crash_video.mp4.tmp");

	// Remove any stale artefacts from previous runs so we never attach a video
	// older than this session.
	IFileManager::Get().Delete(*AttachmentPath, /*RequireExists*/ false, /*EvenReadOnly*/ true);
	IFileManager::Get().Delete(*TempPath, /*RequireExists*/ false, /*EvenReadOnly*/ true);
	bSnapshotOnDisk.AtomicSet(false);

	uint32 ConfiguredWidth = 0, ConfiguredHeight = 0;
	GetResolutionExtents(Settings->CrashVideoResolution, ConfiguredWidth, ConfiguredHeight);
	// v1: ignore configured target resolution and capture native backbuffer size.
	// Downscaling requires a draw pass; deferred to a follow-up. The encoder
	// picks Width/Height from the first frame.
	(void)ConfiguredWidth; (void)ConfiguredHeight;

	Encoder = MakeUnique<FSentryVideoEncoder>(
		*this,
		/*Width*/ 0,
		/*Height*/ 0,
		static_cast<uint32>(Settings->CrashVideoFramerate),
		Settings->CrashVideoBitrateKbps,
		Settings->CrashVideoFragmentSeconds);
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
	RotationThread = FRunnableThread::Create(this, TEXT("SentryCrashVideoRotation"), 0, TPri_BelowNormal);
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
	UE_LOG(LogSentrySdk, Log, TEXT("Crash video enabled, attachment path: %s"), *AttachmentPath);
	return true;
}

void FSentryCrashVideoSubsystem::Shutdown()
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

void FSentryCrashVideoSubsystem::OnInitSegmentReady(TArray<uint8>&& NewInitSegment)
{
	FScopeLock Lock(&RingLock);
	InitSegment = MoveTemp(NewInitSegment);
}

void FSentryCrashVideoSubsystem::OnFragmentReady(TArray<uint8>&& Fragment)
{
	FScopeLock Lock(&RingLock);
	if (FragmentRing.Num() >= FragmentRingCapacity)
	{
		FragmentRing.PopFront();
	}
	FragmentRing.Add(MoveTemp(Fragment));
}

bool FSentryCrashVideoSubsystem::Init()
{
	return true;
}

uint32 FSentryCrashVideoSubsystem::Run()
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

void FSentryCrashVideoSubsystem::Stop()
{
	bStopRequested.AtomicSet(true);
	if (RotationWake)
	{
		RotationWake->Trigger();
	}
}

void FSentryCrashVideoSubsystem::Exit()
{
}

void FSentryCrashVideoSubsystem::DoRotation()
{
	// Read the tfdt's base_media_decode_time from a serialised fragment. The
	// field lives at a fixed offset inside our fragment layout:
	//   moof box header (8) + mfhd (16) + traf box header (8)
	//   + tfhd (16) + tfdt size+type+ver+flags (12) = 60.
	// And it's a big-endian 64-bit value (we always emit tfdt v1).
	constexpr int32 TfdtFieldOffset = 60;
	auto ReadTfdt = [&](const TArray<uint8>& Bytes) -> uint64
	{
		if (Bytes.Num() < TfdtFieldOffset + 8) return 0;
		uint64 V = 0;
		for (int32 b = 0; b < 8; ++b)
		{
			V = (V << 8) | static_cast<uint64>(Bytes[TfdtFieldOffset + b]);
		}
		return V;
	};
	auto WriteTfdtAt = [&](TArray<uint8>& Bytes, int32 Offset, uint64 V)
	{
		if (Bytes.Num() < Offset + 8) return;
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

bool FSentryCrashVideoSubsystem::WriteSnapshotAtomically(const TArray<uint8>& Bytes)
{
	// 1) Write bytes into the temp file (truncate any prior remnants).
	{
		TUniquePtr<FArchive> Out(IFileManager::Get().CreateFileWriter(*TempPath, FILEWRITE_EvenIfReadOnly));
		if (!Out)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: failed to open %s for write"), *TempPath);
			return false;
		}
		Out->Serialize(const_cast<uint8*>(Bytes.GetData()), Bytes.Num());
		Out->Flush();
		if (!Out->Close())
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: failed to close temp file"));
			return false;
		}
	}

	// 2) Atomic rename.
	//    On Windows we use SetFileInformationByHandle(FileRenameInfoEx) with
	//    POSIX semantics, which succeeds even if the destination is open by
	//    another process (e.g. a video player previewing the file). Available
	//    on Win10 1607+. Falls back to MoveFileExW if the POSIX path fails.
#if PLATFORM_WINDOWS
	bool bMoved = false;
	DWORD PosixErr = 0;
	DWORD CreateErr = 0;
	{
		// Convert UE-style forward slashes to backslashes for Win32 APIs that
		// are picky about path separators (FileRenameInfoEx in particular).
		FString WinTempPath = TempPath;
		FString WinAttachmentPath = AttachmentPath;
		WinTempPath.ReplaceInline(TEXT("/"), TEXT("\\"));
		WinAttachmentPath.ReplaceInline(TEXT("/"), TEXT("\\"));

		const HANDLE hSrc = ::CreateFileW(
			*WinTempPath,
			DELETE | SYNCHRONIZE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);
		if (hSrc == INVALID_HANDLE_VALUE)
		{
			CreateErr = ::GetLastError();
		}
		else
		{
			const int32 NameLenChars = WinAttachmentPath.Len();
			const SIZE_T NameBytes = static_cast<SIZE_T>(NameLenChars) * sizeof(WCHAR);
			const SIZE_T BufBytes = sizeof(FILE_RENAME_INFO) + NameBytes;

			TArray<uint8> Buf;
			Buf.SetNumZeroed(static_cast<int32>(BufBytes));
			FILE_RENAME_INFO* Info = reinterpret_cast<FILE_RENAME_INFO*>(Buf.GetData());
			// The struct exposes ReplaceIfExists as a BOOLEAN unioned with a DWORD
			// Flags field used for FileRenameInfoEx. Write the Flags via the
			// union's first byte and overlay the remaining bits explicitly.
			constexpr DWORD RenameFlags =
				/*FILE_RENAME_FLAG_REPLACE_IF_EXISTS*/ 0x1 |
				/*FILE_RENAME_FLAG_POSIX_SEMANTICS  */ 0x2;
			*reinterpret_cast<DWORD*>(&Info->ReplaceIfExists) = RenameFlags;
			Info->RootDirectory = nullptr;
			Info->FileNameLength = static_cast<DWORD>(NameBytes);
			FMemory::Memcpy(Info->FileName, *WinAttachmentPath, NameBytes);

			// FileRenameInfoEx (value 22) — present in Win10 1607+ SDK but the
			// FILE_INFO_BY_HANDLE_CLASS enum constant isn't visible at our
			// _WIN32_WINNT level. Use the numeric value directly.
			constexpr FILE_INFO_BY_HANDLE_CLASS InfoClass_FileRenameInfoEx = static_cast<FILE_INFO_BY_HANDLE_CLASS>(22);
			if (::SetFileInformationByHandle(hSrc, InfoClass_FileRenameInfoEx, Info, static_cast<DWORD>(BufBytes)))
			{
				bMoved = true;
			}
			else
			{
				PosixErr = ::GetLastError();
			}
			::CloseHandle(hSrc);
		}
	}
	if (!bMoved)
	{
		// Fallback: classic atomic rename. Works as long as the dest isn't
		// held with a sharing mode that blocks renames.
		bMoved = !!::MoveFileExW(
			*TempPath,
			*AttachmentPath,
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		if (!bMoved)
		{
			const DWORD MoveErr = ::GetLastError();
			static bool bLoggedOnce = false;
			if (!bLoggedOnce)
			{
				if (PosixErr == /*ERROR_SHARING_VIOLATION*/ 32 || MoveErr == /*ERROR_ACCESS_DENIED*/ 5)
				{
					UE_LOG(LogSentrySdk, Warning,
						TEXT("Crash video: cannot rotate %s — another process is holding it open (e.g. a video preview window). ")
						TEXT("Rotation will keep retrying. In a normal crash scenario this never happens because nothing else has the file open."),
						*AttachmentPath);
				}
				else
				{
					UE_LOG(LogSentrySdk, Warning,
						TEXT("Crash video: rename failed. CreateFile err=%u, SetFileInformationByHandle err=%u, MoveFileExW err=%u"),
						CreateErr, PosixErr, MoveErr);
				}
				bLoggedOnce = true;
			}
			return false;
		}
	}
#else
	if (!IFileManager::Get().Move(*AttachmentPath, *TempPath, /*Replace*/ true, /*EvenIfReadOnly*/ true))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash video: rename to %s failed"), *AttachmentPath);
		return false;
	}
#endif
	return true;
}

#endif // USE_SENTRY_CRASH_VIDEO
