// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryPerformanceMetricsIntegration.h"

#include "SentrySettings.h"

#include "Interface/SentrySubsystemInterface.h"

#include "Performance/SentryPerfFrameTimeMonitor.h"
#include "Performance/SentryPerfGCMonitor.h"
#include "Performance/SentryPerfGameStatsMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"
#include "Performance/SentryPerfNetworkMonitor.h"

#include "Engine/Engine.h"

FSentryPerformanceMetricsIntegration::FSentryPerformanceMetricsIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

bool FSentryPerformanceMetricsIntegration::Setup(const USentrySettings& Settings)
{
	const bool bFrameTime = Settings.EnableAutoFrameTimeMetrics;
	const bool bGameStats = Settings.EnableAutoGameStatsMetrics;

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	const bool bGC = Settings.EnableAutoGCMetrics;
#else
	const bool bGC = false;
#endif

#if !UE_VERSION_OLDER_THAN(5, 7, 0)
	const bool bNetwork = Settings.EnableAutoNetworkMetrics;
#else
	const bool bNetwork = false;
#endif

	if (!bFrameTime && !bGameStats && !bGC && !bNetwork)
	{
		return false;
	}

	MetricAttributes = MakeShared<FSentryPerfMetricAttributes>();

	if (bFrameTime)
	{
		FrameTimeMonitor = MakeShared<FSentryPerfFrameTimeMonitor>(NativeImpl, MetricAttributes, Settings.FrameTimeSampleInterval);

		if (GEngine)
		{
			GEngine->AddPerformanceDataConsumer(FrameTimeMonitor);
		}
	}

	if (bGameStats)
	{
		GameStatsMonitor = MakeShared<FSentryPerfGameStatsMonitor>(NativeImpl, MetricAttributes, Settings.GameStatsSampleInterval);
	}

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	if (bGC)
	{
		GCMonitor = MakeShared<FSentryPerfGCMonitor>(NativeImpl, MetricAttributes);
	}
#endif

#if !UE_VERSION_OLDER_THAN(5, 7, 0)
	if (bNetwork)
	{
		NetworkMonitor = MakeShared<FSentryPerfNetworkMonitor>(NativeImpl, MetricAttributes, Settings.NetworkMetricsSampleInterval);
	}
#endif

	return true;
}

void FSentryPerformanceMetricsIntegration::Teardown()
{
	if (FrameTimeMonitor.IsValid() && GEngine)
	{
		GEngine->RemovePerformanceDataConsumer(FrameTimeMonitor);
	}

	FrameTimeMonitor.Reset();
	GameStatsMonitor.Reset();

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	GCMonitor.Reset();
#endif

#if !UE_VERSION_OLDER_THAN(5, 7, 0)
	NetworkMonitor.Reset();
#endif

	MetricAttributes.Reset();
}
