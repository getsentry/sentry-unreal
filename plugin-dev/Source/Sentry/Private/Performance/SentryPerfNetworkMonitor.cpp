// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfNetworkMonitor.h"

#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5, 4, 0)

#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Net/NetworkMetricsDatabase.h"
#include "Net/NetworkMetricsDefs.h"

const TArray<FSentryNetworkMetricMapping> FSentryPerfNetworkMonitor::MetricMappings = {
	{ UE::Net::Metric::InRate, TEXT("game.perf.net.in_rate"), ESentryUnit::Byte, false },
	{ UE::Net::Metric::OutRate, TEXT("game.perf.net.out_rate"), ESentryUnit::Byte, false },
	{ UE::Net::Metric::InPackets, TEXT("game.perf.net.in_packets"), ESentryUnit::None, false },
	{ UE::Net::Metric::OutPackets, TEXT("game.perf.net.out_packets"), ESentryUnit::None, false },
	{ UE::Net::Metric::InPacketsLost, TEXT("game.perf.net.in_packets_lost"), ESentryUnit::None, false },
	{ UE::Net::Metric::OutPacketsLost, TEXT("game.perf.net.out_packets_lost"), ESentryUnit::None, false },
	{ UE::Net::Metric::Ping, TEXT("game.perf.net.ping"), ESentryUnit::Millisecond, false },
	{ UE::Net::Metric::AverageJitterInMS, TEXT("game.perf.net.jitter"), ESentryUnit::Millisecond, true },
	{ UE::Net::Metric::NumClients, TEXT("game.perf.net.num_clients"), ESentryUnit::None, false },
	{ UE::Net::Metric::RawPingClientAvg, TEXT("game.perf.net.ping_client_avg"), ESentryUnit::Millisecond, true },
	{ UE::Net::Metric::RawPingClientMax, TEXT("game.perf.net.ping_client_max"), ESentryUnit::Millisecond, false },
	{ UE::Net::Metric::InRateClientAvg, TEXT("game.perf.net.in_rate_client_avg"), ESentryUnit::Byte, false },
	{ UE::Net::Metric::OutRateClientAvg, TEXT("game.perf.net.out_rate_client_avg"), ESentryUnit::Byte, false },
	{ UE::Net::Metric::SatConnections, TEXT("game.perf.net.sat_connections"), ESentryUnit::None, false },
};

FSentryPerfNetworkMonitor::FSentryPerfNetworkMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes)
	: MetricAttributes(InMetricAttributes)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	const float IntervalSeconds = static_cast<float>(FMath::Max(Settings->NetworkMetricsSampleInterval, 1));

	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("SentryPerfNetworkMonitor"), IntervalSeconds, [this](float DeltaTime)
	{
		return OnTick(DeltaTime);
	});

	UE_LOG(LogSentrySdk, Log, TEXT("Network metrics monitor started (interval: %.0fs)."), IntervalSeconds);
}

FSentryPerfNetworkMonitor::~FSentryPerfNetworkMonitor()
{
	FTSTicker::RemoveTicker(TickerHandle);
}

void FSentryPerfNetworkMonitor::SetNetDriver(UNetDriver* InNetDriver)
{
	WeakNetDriver = InNetDriver;

	if (InNetDriver)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Network metrics monitor attached to NetDriver '%s'."), *InNetDriver->GetName());
	}
}

bool FSentryPerfNetworkMonitor::OnTick(float DeltaTime)
{
	UNetDriver* NetDriver = WeakNetDriver.Get();
	if (!NetDriver)
	{
		return true;
	}

	UNetworkMetricsDatabase* MetricsDb = NetDriver->GetMetrics();
	if (!MetricsDb)
	{
		return true;
	}

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return true;
	}

	const TMap<FString, FSentryVariant>& Attributes = MetricAttributes->GetAttributes();

	for (const FSentryNetworkMetricMapping& Mapping : MetricMappings)
	{
		if (!MetricsDb->Contains(Mapping.UEName))
		{
			continue;
		}

		const float Value = Mapping.bIsFloat
								? MetricsDb->GetFloat(Mapping.UEName)
								: static_cast<float>(MetricsDb->GetInt(Mapping.UEName));

		Sentry->AddGaugeWithAttributes(Mapping.SentryKey, Value, FSentryUnit(Mapping.Unit), Attributes);
	}

	return true;
}

#endif // !UE_VERSION_OLDER_THAN(5, 4, 0)
