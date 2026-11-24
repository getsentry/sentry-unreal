// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfGCMonitor.h"

#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5, 5, 0)

#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryDefines.h"
#include "SentryUnit.h"

#include "Interface/SentrySubsystemInterface.h"

#include "HAL/PlatformTime.h"
#include "UObject/UObjectGlobals.h"

FSentryPerfGCMonitor::FSentryPerfGCMonitor(const TSharedRef<ISentrySubsystem>& InNativeImpl, TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes)
	: GCStartTime(0.0)
	, NativeImpl(InNativeImpl)
	, MetricAttributes(InMetricAttributes)
{
	// GetGarbageCollectStartedDelegate fires after the GC lock is acquired, measuring only the actual GC pause
	GCStartedHandle = FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().AddRaw(this, &FSentryPerfGCMonitor::OnGCStarted);
	PostGCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FSentryPerfGCMonitor::OnPostGC);

	UE_LOG(LogSentrySdk, Log, TEXT("GC monitor started."));
}

FSentryPerfGCMonitor::~FSentryPerfGCMonitor()
{
	FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().Remove(GCStartedHandle);
	FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGCHandle);
}

void FSentryPerfGCMonitor::OnGCStarted()
{
	GCStartTime = FPlatformTime::Seconds();
}

void FSentryPerfGCMonitor::OnPostGC()
{
	const float PauseMs = static_cast<float>((FPlatformTime::Seconds() - GCStartTime) * 1000.0);

	NativeImpl->AddDistribution(TEXT("game.perf.gc_time"), PauseMs, FSentryUnit(ESentryUnit::Millisecond).ToString(), MetricAttributes->GetAttributes());
}

#endif // !UE_VERSION_OLDER_THAN(5, 5, 0)
