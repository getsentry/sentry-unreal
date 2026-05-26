// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if defined(USE_SENTRY_SESSION_REPLAY)

#include "Containers/RingBuffer.h"
#include "HAL/CriticalSection.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

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

	bool Initialize(const USentrySettings* Settings, const FString& ReplayPath);
	void Shutdown();

	bool IsEnabled() const { return bEnabled; }
	bool HasSnapshotOnDisk() const { return bSnapshotOnDisk; }
	const FString& GetAttachmentPath() const { return AttachmentPath; }

	// Called by the encoder thread when the init segment (ftyp+moov) is ready
	void OnInitSegmentReady(TArray<uint8>&& InitSegment);
	// Called by the encoder thread when a fragment (moof+mdat) is complete
	void OnFragmentReady(TArray<uint8>&& Fragment);

	// FRunnable (rotation thread)
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:
	void DoRotation();
	bool WriteSnapshot(const TArray<uint8>& Bytes);

	bool bEnabled = false;
	FThreadSafeBool bSnapshotOnDisk;
	FThreadSafeBool bStopRequested;

	FString AttachmentPath;
	FString TempPath;

	float WindowSeconds = 12.0f;
	float FragmentSeconds = 0.5f;
	float RotationIntervalSeconds = 1.0f;

	int32 FragmentRingCapacity = 24;

	TUniquePtr<FSentryVideoEncoder> Encoder;
	TUniquePtr<FSentryBackBufferCapture> Capture;

	// Fragment ring + init segment, protected by RingLock
	FCriticalSection RingLock;
	TArray<uint8> InitSegment;
	TRingBuffer<TArray<uint8>> FragmentRing;

	// Rotation thread
	FRunnableThread* RotationThread = nullptr;
	FEvent* RotationWake = nullptr;
};

#endif // USE_SENTRY_SESSION_REPLAY
