// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"

DECLARE_DELEGATE_TwoParams(FOnHangDetected, uint32 /*HungThreadId*/, double /*HangDuration*/);

/**
 * Watches for thread hangs using the engine's FThreadHeartBeat system.
 *
 * Binds to OnThreadStuck/OnThreadUnstuck delegates. When a thread is reported stuck,
 * a watcher thread waits the remaining time (HangTimeout - StuckDuration) before
 * firing the OnHangDetected delegate. If the thread recovers before the timeout,
 * the capture is cancelled.
 *
 * Requires HangDuration > 0 in [Core.System] engine config to enable the heartbeat monitor thread.
 */
class FSentryHangWatcher : public FRunnable
{
public:
	FSentryHangWatcher(float InHangTimeoutSeconds);
	virtual ~FSentryHangWatcher();

	/** Binds to heartbeat delegates and starts the watcher thread. */
	void Start();

	/** Unbinds delegates and stops the watcher thread. */
	void Stop();

	/** Delegate fired when a hang is detected. Called from the watcher thread. */
	FOnHangDetected OnHangDetected;

	// FRunnable interface
	virtual uint32 Run() override;

private:
	void OnThreadStuck(uint32 ThreadId);
	void OnThreadUnstuck(uint32 ThreadId);

	float HangTimeoutSeconds;
	float EngineStuckDuration;

	TAtomic<uint32> StuckThreadId;
	FThreadSafeBool bRunning;

	FRunnableThread* WatcherThread;
	FEvent* WakeEvent;
};
