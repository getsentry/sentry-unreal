// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfGCMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryDefines.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectGlobals.h"

FSentryPerfGCMonitor::FSentryPerfGCMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes)
	: GCStartTime(0.0)
	, MetricAttributes(InMetricAttributes)
{
	// GetGarbageCollectStartedDelegate (5.5+) fires after the GC lock is acquired, measuring only the actual
	// GC pause. GetPreGarbageCollectDelegate fires before lock acquisition, so on older engine versions the
	// measurement may include time spent waiting for async threads to release the GC lock.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
	GCStartedHandle = FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddRaw(this, &FSentryPerfGCMonitor::OnGCStarted);
#else
	GCStartedHandle = FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().AddRaw(this, &FSentryPerfGCMonitor::OnGCStarted);
#endif
	PostGCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FSentryPerfGCMonitor::OnPostGC);

	UE_LOG(LogSentrySdk, Log, TEXT("GC monitor started."));
}

FSentryPerfGCMonitor::~FSentryPerfGCMonitor()
{
	if (GCStartedHandle.IsValid())
	{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		FCoreUObjectDelegates::GetPreGarbageCollectDelegate().Remove(GCStartedHandle);
#else
		FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().Remove(GCStartedHandle);
#endif
		GCStartedHandle.Reset();
	}

	if (PostGCHandle.IsValid())
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGCHandle);
		PostGCHandle.Reset();
	}
}

void FSentryPerfGCMonitor::OnGCStarted()
{
	GCStartTime = FPlatformTime::Seconds();
}

void FSentryPerfGCMonitor::OnPostGC()
{
	const float PauseMs = static_cast<float>((FPlatformTime::Seconds() - GCStartTime) * 1000.0);

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return;
	}

	Sentry->AddDistributionWithAttributes(TEXT("game.perf.gc_time"), PauseMs, FSentryUnit(ESentryUnit::Millisecond), MetricAttributes->GetAttributes());
}
