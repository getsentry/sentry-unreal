// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Engine/NetDriver.h"
#include "Misc/EngineVersionComparison.h"
#include "SentryUnit.h"

#if !UE_VERSION_OLDER_THAN(5, 7, 0)

class FSentryPerfMetricAttributes;

struct FSentryNetworkMetricMapping
{
	FName UEName;
	const TCHAR* SentryKey;
	ESentryUnit Unit;
	bool bIsFloat;
};

/**
 * Periodically polls network metrics from UNetDriver's UNetworkMetricsDatabase and emits them as Sentry metrics.
 *
 * The NetDriver reference is set reactively via SetNetDriver() when FWorldDelegates::OnNetDriverCreated fires.
 * Uses a ticker to poll on a configurable interval (default 10 seconds).
 *
 * Emits (when available):
 * - game.perf.net.in_rate / out_rate (gauge, bytes/sec) — bandwidth
 * - game.perf.net.in_packets / out_packets (gauge) — packet throughput
 * - game.perf.net.in_packets_lost / out_packets_lost (gauge) — packet loss
 * - game.perf.net.ping (gauge, milliseconds) — client ping
 * - game.perf.net.jitter (gauge, milliseconds) — client jitter
 * - game.perf.net.num_clients (gauge) — server client count
 * - game.perf.net.ping_client_avg / ping_client_max (gauge, milliseconds) — server ping stats
 * - game.perf.net.in_rate_client_avg / out_rate_client_avg (gauge, bytes/sec) — server per-client bandwidth
 * - game.perf.net.sat_connections (gauge) — saturated connections
 */
class FSentryPerfNetworkMonitor
{
public:
	explicit FSentryPerfNetworkMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes);
	~FSentryPerfNetworkMonitor();

	void SetNetDriver(UNetDriver* InNetDriver);

private:
	bool OnTick(float DeltaTime);

	TSharedPtr<FSentryPerfMetricAttributes> MetricAttributes;
	TWeakObjectPtr<UNetDriver> WeakNetDriver;
	FTSTicker::FDelegateHandle TickerHandle;

	static const TArray<FSentryNetworkMetricMapping> MetricMappings;
};

#endif // !UE_VERSION_OLDER_THAN(5, 7, 0)
