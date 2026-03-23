// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryGCListener.h"

#include "SentryDefines.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"
#include "Utils/SentryPerformanceMetricAttributes.h"

#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "UObject/UObjectGlobals.h"

FSentryGCListener::FSentryGCListener(TSharedPtr<FSentryPerformanceMetricAttributes> InMetricAttributes)
	: GCStartTime(0.0)
	, MetricAttributes(InMetricAttributes)
{
	GCStartedHandle = FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().AddRaw(this, &FSentryGCListener::OnGCStarted);
	PostGCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FSentryGCListener::OnPostGC);

	UE_LOG(LogSentrySdk, Log, TEXT("GC listener started."));
}

FSentryGCListener::~FSentryGCListener()
{
	if (GCStartedHandle.IsValid())
	{
		FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().Remove(GCStartedHandle);
		GCStartedHandle.Reset();
	}

	if (PostGCHandle.IsValid())
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGCHandle);
		PostGCHandle.Reset();
	}
}

void FSentryGCListener::OnGCStarted()
{
	GCStartTime = FPlatformTime::Seconds();
}

void FSentryGCListener::OnPostGC()
{
	if (GCStartTime <= 0.0)
	{
		return;
	}

	const float PauseMs = static_cast<float>((FPlatformTime::Seconds() - GCStartTime) * 1000.0);

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return;
	}

	Sentry->AddDistributionWithAttributes(TEXT("game.perf.gc_time"), PauseMs, FSentryUnit(ESentryUnit::Millisecond), MetricAttributes->GetAttributes());
}
