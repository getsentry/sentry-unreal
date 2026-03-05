// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryHangWatcher.h"

#include "SentryDefines.h"

#include "HAL/Event.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadHeartBeat.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersionComparison.h"

static const uint32 InvalidThreadId = (uint32)-1;

FSentryHangWatcher::FSentryHangWatcher(float InHangTimeoutSeconds)
	: HangTimeoutSeconds(InHangTimeoutSeconds)
	, EngineStuckDuration(1.0f)
	, StuckThreadId(InvalidThreadId)
	, bRunning(false)
	, WatcherThread(nullptr)
	, WakeEvent(nullptr)
{
	WakeEvent = FPlatformProcess::GetSynchEventFromPool(false);

	// Read engine's StuckDuration from config (same source as FThreadHeartBeat)
	double ConfigStuckDuration = 1.0;
	if (GConfig)
	{
		GConfig->GetDouble(TEXT("Core.System"), TEXT("StuckDuration"), ConfigStuckDuration, GEngineIni);
	}
	EngineStuckDuration = (float)ConfigStuckDuration;
}

FSentryHangWatcher::~FSentryHangWatcher()
{
	Stop();

	if (WakeEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
		WakeEvent = nullptr;
	}
}

void FSentryHangWatcher::Start()
{
	if (bRunning)
	{
		return;
	}

	FThreadHeartBeat& HeartBeat = FThreadHeartBeat::Get();

	const double HangDuration = HeartBeat.GetHangDuration();
	if (HangDuration <= 0.0)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Engine's HangDuration is 0 — heartbeat monitor thread is not running. Set HangDuration > 0 in [Core.System] to enable hang tracking."));
		return;
	}

	if (HangTimeoutSeconds < EngineStuckDuration)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Hang timeout (%.1fs) is less than engine's StuckDuration (%.1fs). Adjusting to %.1fs."),
			HangTimeoutSeconds, EngineStuckDuration, EngineStuckDuration);
		HangTimeoutSeconds = EngineStuckDuration;
	}

	// In UE 5.3 and older, OnStuck/OnUnstuck delegates only fire when the
	// AttemptStuckThreadResuscitation CVar is true (defaults to false).
	// UE 5.4+ removed this gate entirely. Enable it so delegates fire.
#if UE_VERSION_OLDER_THAN(5, 4, 0)
	IConsoleVariable* ResuscitationCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("AttemptStuckThreadResuscitation"));
	if (ResuscitationCVar)
	{
		ResuscitationCVar->Set(true);
	}
#endif

	bRunning = true;

	HeartBeat.GetOnThreadStuck().BindRaw(this, &FSentryHangWatcher::OnThreadStuck);
	HeartBeat.GetOnThreadUnstuck().BindRaw(this, &FSentryHangWatcher::OnThreadUnstuck);

	WatcherThread = FRunnableThread::Create(this, TEXT("SentryHangWatcher"), 0, TPri_BelowNormal);

	UE_LOG(LogSentrySdk, Log, TEXT("Hang watcher started (timeout: %.1fs, engine stuck duration: %.1fs, engine hang duration: %.1fs)."),
		HangTimeoutSeconds, EngineStuckDuration, HangDuration);
}

void FSentryHangWatcher::Stop()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;

	FThreadHeartBeat& HeartBeat = FThreadHeartBeat::Get();
	HeartBeat.GetOnThreadStuck().Unbind();
	HeartBeat.GetOnThreadUnstuck().Unbind();

	if (WakeEvent)
	{
		WakeEvent->Trigger();
	}

	if (WatcherThread)
	{
		WatcherThread->WaitForCompletion();
		delete WatcherThread;
		WatcherThread = nullptr;
	}
}

void FSentryHangWatcher::OnThreadStuck(uint32 ThreadId)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Thread %u reported stuck by engine heartbeat."), ThreadId);

	StuckThreadId = ThreadId;
	WakeEvent->Trigger();
}

void FSentryHangWatcher::OnThreadUnstuck(uint32 ThreadId)
{
	if (StuckThreadId != ThreadId)
	{
		return;
	}

	UE_LOG(LogSentrySdk, Log, TEXT("Thread %u recovered (unstuck)."), ThreadId);

	StuckThreadId = InvalidThreadId;
	WakeEvent->Trigger();
}

uint32 FSentryHangWatcher::Run()
{
	while (bRunning)
	{
		// Wait for a thread to be reported stuck
		WakeEvent->Wait(MAX_uint32);

		if (!bRunning)
		{
			break;
		}

		// A thread was reported stuck — wait the remaining time before capturing
		const float RemainingWaitSeconds = FMath::Max(HangTimeoutSeconds - EngineStuckDuration, 0.0f);
		const uint32 RemainingWaitMs = (uint32)(RemainingWaitSeconds * 1000.0f);

		if (RemainingWaitMs > 0)
		{
			WakeEvent->Wait(RemainingWaitMs);
		}

		if (!bRunning)
		{
			break;
		}

		// Check if the thread is still stuck
		const uint32 CurrentStuckThread = StuckThreadId;
		if (CurrentStuckThread != InvalidThreadId)
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Thread %u hang detected (unresponsive for %.1fs)."), CurrentStuckThread, HangTimeoutSeconds);

			OnHangDetected.ExecuteIfBound(CurrentStuckThread, (double)HangTimeoutSeconds);

			// Wait until this stuck episode resolves before watching for the next one
			while (bRunning && StuckThreadId != InvalidThreadId)
			{
				WakeEvent->Wait(1000);
			}
		}
	}

	return 0;
}
