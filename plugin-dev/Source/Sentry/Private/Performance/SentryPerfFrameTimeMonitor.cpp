// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfFrameTimeMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryUnit.h"

#include "Interface/SentrySubsystemInterface.h"

#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"

extern ENGINE_API float GAverageFPS;

FSentryPerfFrameTimeMonitor::FSentryPerfFrameTimeMonitor(const TSharedRef<ISentrySubsystem>& InNativeImpl, TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes, float InSampleIntervalSeconds)
	: SampleIntervalSeconds(FMath::Max(static_cast<double>(InSampleIntervalSeconds), 0.05))
	, LastEmitTime(0.0)
	, NativeImpl(InNativeImpl)
	, MetricAttributes(InMetricAttributes)
{
}

void FSentryPerfFrameTimeMonitor::StartCharting()
{
}

void FSentryPerfFrameTimeMonitor::ProcessFrame(const FFrameData& FrameData)
{
	const double Now = FPlatformTime::Seconds();
	if (Now - LastEmitTime < SampleIntervalSeconds)
	{
		return;
	}
	LastEmitTime = Now;

	const FString Ms = FSentryUnit(ESentryUnit::Millisecond).ToString();
	const TMap<FString, FSentryVariant>& Attributes = MetricAttributes->GetAttributes();

	NativeImpl->AddDistribution(TEXT("game.perf.frame_time"), FrameData.TrueDeltaSeconds * 1000.0f, Ms, Attributes);
	NativeImpl->AddDistribution(TEXT("game.perf.game_thread"), FrameData.GameThreadTimeSeconds * 1000.0f, Ms, Attributes);
	NativeImpl->AddDistribution(TEXT("game.perf.render_thread"), FrameData.RenderThreadTimeSeconds * 1000.0f, Ms, Attributes);
	NativeImpl->AddDistribution(TEXT("game.perf.gpu"), FrameData.GPUTimeSeconds * 1000.0f, Ms, Attributes);

	NativeImpl->AddGauge(TEXT("game.perf.fps"), GAverageFPS, FSentryUnit(ESentryUnit::None).ToString(), Attributes);
}

void FSentryPerfFrameTimeMonitor::StopCharting()
{
}
