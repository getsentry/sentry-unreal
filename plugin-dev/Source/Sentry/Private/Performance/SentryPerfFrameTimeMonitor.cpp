// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfFrameTimeMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"

extern ENGINE_API float GAverageFPS;

FSentryPerfFrameTimeMonitor::FSentryPerfFrameTimeMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes)
	: SampleIntervalSeconds(1.0)
	, LastEmitTime(0.0)
	, MetricAttributes(InMetricAttributes)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	SampleIntervalSeconds = FMath::Max(static_cast<double>(Settings->FrameTimeSampleInterval), 0.05);
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

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return;
	}

	const FSentryUnit Ms(ESentryUnit::Millisecond);
	const TMap<FString, FSentryVariant>& Attributes = MetricAttributes->GetAttributes();

	Sentry->AddDistributionWithAttributes(TEXT("game.perf.frame_time"), FrameData.TrueDeltaSeconds * 1000.0f, Ms, Attributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.game_thread"), FrameData.GameThreadTimeSeconds * 1000.0f, Ms, Attributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.render_thread"), FrameData.RenderThreadTimeSeconds * 1000.0f, Ms, Attributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.gpu"), FrameData.GPUTimeSeconds * 1000.0f, Ms, Attributes);

	Sentry->AddGaugeWithAttributes(TEXT("game.perf.fps"), GAverageFPS, FSentryUnit(ESentryUnit::None), Attributes);
}

void FSentryPerfFrameTimeMonitor::StopCharting()
{
}
