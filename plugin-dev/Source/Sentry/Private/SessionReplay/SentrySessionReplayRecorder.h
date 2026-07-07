// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#ifdef USE_SENTRY_SESSION_REPLAY

#include "Containers/RingBuffer.h"
#include "HAL/CriticalSection.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

#include "SentryReplayInfo.h"

class FRunnableThread;
class FEvent;
class FSentryVideoEncoder;
class FSentryBackBufferCapture;
class USentrySettings;

/**
 * Owns the session-replay capture pipeline (crash-attached gameplay video).
 *
 * Threads:
 *   - Render thread: backbuffer capture hook copies the swap chain into a
 *     pooled texture and hands it to the encoder.
 *   - Encoder thread (in FSentryVideoEncoder): drains the AVCodecs encoder
 *     and publishes init segment + complete fragments back to us via the
 *     OnInitSegmentReady / OnFragmentReady callbacks below.
 *   - Rotation thread (this class's FRunnable): periodically composes
 *     `init + last N fragments` into a temp file and atomically renames
 *     it into place over the attachment path.
 */
class FSentrySessionReplayRecorder : public FRunnable
{
public:
	FSentrySessionReplayRecorder();
	virtual ~FSentrySessionReplayRecorder() override;

	bool Initialize(const USentrySettings* Settings, const FString& ReplayId, const FString& ReplayPath);
	void Shutdown();

	bool IsEnabled() const { return bEnabled; }
	bool HasSnapshotOnDisk() const { return bSnapshotOnDisk; }
	const FString& GetAttachmentPath() const { return AttachmentPath; }

	// Called by the encoder thread when the init segment (ftyp+moov) is ready
	void OnInitSegmentReady(TArray<uint8>&& InitSegment);
	// Called by the encoder thread when a fragment (moof+mdat) is complete
	void OnFragmentReady(TArray<uint8>&& Fragment, uint32 FrameCount, uint64 DurationTicks);

	// FRunnable (rotation thread)
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	void DoRotation();
	bool WriteSnapshot(const TArray<uint8>& Bytes);

	// Assembles the platform-agnostic replay model from the most recent on-disk snapshot
	FSentryReplayInfo BuildReplayInfo() const;

	// Writes the JSON metadata sidecar next to the mp4 (temp + atomic rename) so the
	// SDK can build and send the replay envelope outside the crash handler
	void WriteReplayMetadata();

	bool bEnabled = false;
	FThreadSafeBool bSnapshotOnDisk;
	FThreadSafeBool bStopRequested;

	FString ReplayId;
	FString AttachmentPath;
	FString TempPath;
	FString MetadataPath;
	FString MetadataTempPath;

	float WindowSeconds = 12.0f;
	float FragmentSeconds = 0.5f;
	float RotationIntervalSeconds = 1.0f;

	int32 FragmentRingCapacity = 24;

	TUniquePtr<FSentryVideoEncoder> Encoder;
	TUniquePtr<FSentryBackBufferCapture> Capture;

	struct FFragment
	{
		TArray<uint8> Bytes;
		uint32 FrameCount = 0;
		uint64 DurationTicks = 0;
	};

	// Fragment ring + init segment, protected by RingLock
	FCriticalSection RingLock;
	TArray<uint8> InitSegment;
	TRingBuffer<FFragment> FragmentRing;

	int32 LatestFrameCount = 0;
	int64 LatestDurationMs = 0;

	// Rotation thread
	FRunnableThread* RotationThread = nullptr;
	FEvent* RotationWake = nullptr;
};

#endif // USE_SENTRY_SESSION_REPLAY
