// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "HAL/Runnable.h"

#include "HAL/ThreadSafeBool.h"
#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5, 0, 0)

DECLARE_DELEGATE_TwoParams(FOnHangDetected, uint32 /*HungThreadId*/, double /*HangDuration*/);

/**
 * Watches for game thread hangs using a self-contained ticker-based watchdog.
 *
 * Registers an FTSTicker callback on the game thread that updates a heartbeat timestamp
 * every tick. A background watcher thread periodically checks whether the timestamp is stale.
 * If the game thread hasn't ticked for longer than the configured timeout, the OnHangDetected
 * delegate is fired.
 */
class FSentryHangWatcher : public FRunnable
{
public:
	FSentryHangWatcher(float InHangTimeoutSeconds);
	virtual ~FSentryHangWatcher();

	/** Registers the ticker callback and starts the watcher thread. */
	void Start();

	/** Removes the ticker callback and stops the watcher thread. */
	void Stop();

	/** Delegate fired when a hang is detected. Called from the watcher thread. */
	FOnHangDetected OnHangDetected;

	// FRunnable interface
	virtual uint32 Run() override;

private:
	float HangTimeoutSeconds;

	TAtomic<double> LastHeartbeatTime;
	FThreadSafeBool bRunning;

	FTSTicker::FDelegateHandle TickerHandle;
	FRunnableThread* WatcherThread;
	FEvent* WakeEvent;
};

#endif
