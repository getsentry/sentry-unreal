// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfFrameTimeMonitor.h"
#include "Performance/SentryPerfMetricAttributes.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"

extern ENGINE_API float GAverageFPS;

FSentryPerfFrameTimeMonitor::FSentryPerfFrameTimeMonitor(TSharedPtr<FSentryPerfMetricAttributes> InMetricAttributes)
	: SampleInterval(1)
	, FrameCount(0)
	, MetricAttributes(InMetricAttributes)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	SampleInterval = FMath::Max(Settings->FrameTimeSampleInterval, 1);
}

void FSentryPerfFrameTimeMonitor::StartCharting()
{
}

void FSentryPerfFrameTimeMonitor::ProcessFrame(const FFrameData& FrameData)
{
	++FrameCount;

	if (FrameCount % SampleInterval != 0)
	{
		return;
	}

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
