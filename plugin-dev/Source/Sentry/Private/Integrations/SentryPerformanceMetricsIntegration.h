// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

#include "Misc/EngineVersionComparison.h"

class ISentrySubsystem;
class FSentryPerfMetricAttributes;
class FSentryPerfFrameTimeMonitor;
class FSentryPerfGameStatsMonitor;
class FSentryPerfGCMonitor;
class FSentryPerfNetworkMonitor;

/**
 * Emits automatic performance metrics (frame time, game stats, GC pause time, network stats)
 * enabled in plugin settings.
 */
class FSentryPerformanceMetricsIntegration final : public ISentryIntegration
{
public:
	explicit FSentryPerformanceMetricsIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);

	virtual const TCHAR* GetName() const override { return TEXT("PerformanceMetrics"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	TSharedRef<ISentrySubsystem> NativeImpl;

	TSharedPtr<FSentryPerfMetricAttributes> MetricAttributes;

	TSharedPtr<FSentryPerfFrameTimeMonitor> FrameTimeMonitor;
	TSharedPtr<FSentryPerfGameStatsMonitor> GameStatsMonitor;

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	TSharedPtr<FSentryPerfGCMonitor> GCMonitor;
#endif

#if !UE_VERSION_OLDER_THAN(5, 7, 0)
	TSharedPtr<FSentryPerfNetworkMonitor> NetworkMonitor;
#endif
};
