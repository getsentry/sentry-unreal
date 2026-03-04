// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryHangWatcher.h"

#if !UE_VERSION_OLDER_THAN(5, 0, 0)

#include "SentryDefines.h"
#include "Interface/SentrySubsystemInterface.h"

#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"

FSentryHangWatcher::FSentryHangWatcher(TSharedPtr<ISentrySubsystem> InSubsystem, float InHangTimeoutSeconds)
	: SubsystemImpl(InSubsystem)
	, HangTimeoutSeconds(InHangTimeoutSeconds)
	, LastHeartbeatTime(FPlatformTime::Seconds())
	, bRunning(false)
	, WatcherThread(nullptr)
	, WakeEvent(nullptr)
{
	WakeEvent = FPlatformProcess::GetSynchEventFromPool(false);
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

	bRunning = true;

	// Register a ticker callback on the game thread that updates our heartbeat timestamp every tick
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
		[this](float DeltaTime) -> bool
		{
			LastHeartbeatTime = FPlatformTime::Seconds();
			return true;
		}));

	WatcherThread = FRunnableThread::Create(this, TEXT("SentryHangWatcher"), 0, TPri_BelowNormal);

	UE_LOG(LogSentrySdk, Log, TEXT("Hang watcher started (timeout: %.1fs)."), HangTimeoutSeconds);
}

void FSentryHangWatcher::Stop()
{
	if (!bRunning)
	{
		return;
	}

	bRunning = false;

	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

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

uint32 FSentryHangWatcher::Run()
{
	// Poll interval — how often we check whether the game thread is still ticking
	const uint32 PollIntervalMs = 1000;

	while (bRunning)
	{
		WakeEvent->Wait(PollIntervalMs);

		if (!bRunning)
		{
			break;
		}

		const double CurrentTime = FPlatformTime::Seconds();
		const double TimeSinceLastHeartbeat = CurrentTime - LastHeartbeatTime;

		if (TimeSinceLastHeartbeat >= HangTimeoutSeconds)
		{
			const uint32 GameThreadId = GGameThreadId;

			UE_LOG(LogSentrySdk, Warning, TEXT("Game thread hang detected (unresponsive for %.1fs). Capturing hang event."),
				TimeSinceLastHeartbeat);

			SubsystemImpl->CaptureHang(GameThreadId);

			// Wait until the game thread recovers before watching for the next hang
			while (bRunning)
			{
				WakeEvent->Wait(PollIntervalMs);

				const double RecoveryTime = FPlatformTime::Seconds();
				if ((RecoveryTime - LastHeartbeatTime) < HangTimeoutSeconds)
				{
					UE_LOG(LogSentrySdk, Log, TEXT("Game thread recovered."));
					break;
				}
			}
		}
	}

	return 0;
}

#endif // !UE_VERSION_OLDER_THAN(5, 0, 0)
