// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfGameStatsMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryDefines.h"
#include "SentryUnit.h"

#include "Interface/SentrySubsystemInterface.h"

#include "HAL/PlatformMemory.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/UObjectArray.h"

FSentryPerfGameStatsMonitor::FSentryPerfGameStatsMonitor(const TSharedRef<ISentrySubsystem>& InNativeImpl, TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes, int32 InSampleIntervalSeconds)
	: NativeImpl(InNativeImpl)
	, MetricAttributes(InMetricAttributes)
{
	const float IntervalSeconds = static_cast<float>(FMath::Max(InSampleIntervalSeconds, 1));

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FSentryPerfGameStatsMonitor::OnTick), IntervalSeconds);
#else
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("SentryPerfGameStatsMonitor"), IntervalSeconds, [this](float DeltaTime)
	{
		return OnTick(DeltaTime);
	});
#endif

	UE_LOG(LogSentrySdk, Log, TEXT("Game stats monitor started (interval: %.0fs)."), IntervalSeconds);
}

FSentryPerfGameStatsMonitor::~FSentryPerfGameStatsMonitor()
{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
#else
	FTSTicker::RemoveTicker(TickerHandle);
#endif
}

bool FSentryPerfGameStatsMonitor::OnTick(float DeltaTime)
{
	const TMap<FString, FSentryVariant>& Attributes = MetricAttributes->GetAttributes();

	const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	NativeImpl->AddGauge(TEXT("game.perf.used_memory"), static_cast<float>(MemStats.UsedPhysical), FSentryUnit(ESentryUnit::Byte).ToString(), Attributes);

#if !UE_VERSION_OLDER_THAN(5, 3, 0)
	const int32 UObjectCount = GUObjectArray.GetObjectArrayNumMinusAvailable();
	NativeImpl->AddGauge(TEXT("game.perf.uobject_count"), static_cast<float>(UObjectCount), FSentryUnit(ESentryUnit::None).ToString(), Attributes);
#endif

	return true;
}
