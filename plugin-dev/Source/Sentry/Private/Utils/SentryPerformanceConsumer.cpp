// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryPerformanceConsumer.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/CoreDelegates.h"
#include "UObject/CoreDelegates.h"
#include "UnrealEngine.h"

extern ENGINE_API float GAverageFPS;

FSentryPerformanceConsumer::FSentryPerformanceConsumer()
	: SampleInterval(1)
	, FrameCount(0)
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	SampleInterval = FMath::Max(Settings->FrameTimeSampleInterval, 1);

	CacheAttributes();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSentryPerformanceConsumer::OnMapLoaded);
}

FSentryPerformanceConsumer::~FSentryPerformanceConsumer()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
}

void FSentryPerformanceConsumer::StartCharting()
{
}

void FSentryPerformanceConsumer::ProcessFrame(const FFrameData& FrameData)
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

	Sentry->AddDistributionWithAttributes(TEXT("game.perf.frame_duration"), FrameData.TrueDeltaSeconds * 1000.0f, Ms, MetricAttributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.game_thread"), FrameData.GameThreadTimeSeconds * 1000.0f, Ms, MetricAttributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.render_thread"), FrameData.RenderThreadTimeSeconds * 1000.0f, Ms, MetricAttributes);
	Sentry->AddDistributionWithAttributes(TEXT("game.perf.gpu"), FrameData.GPUTimeSeconds * 1000.0f, Ms, MetricAttributes);

	Sentry->AddGaugeWithAttributes(TEXT("game.perf.fps"), GAverageFPS, FSentryUnit(ESentryUnit::None), MetricAttributes);
}

void FSentryPerformanceConsumer::StopCharting()
{
}

void FSentryPerformanceConsumer::OnMapLoaded(UWorld* World)
{
	if (World)
	{
		MetricAttributes.Add(TEXT("map"), World->GetMapName());
	}
}

void FSentryPerformanceConsumer::CacheAttributes()
{
	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());
	if (GpuDriverInfo.IsValid())
	{
		MetricAttributes.Add(TEXT("gpu.name"), GpuDriverInfo.DeviceDescription);
	}

	MetricAttributes.Add(TEXT("cpu.cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	MetricAttributes.Add(TEXT("ram.gb"), FString::FromInt(FPlatformMemory::GetConstants().TotalPhysicalGB));
	MetricAttributes.Add(TEXT("res.x"), FString::FromInt(GSystemResolution.ResX));
	MetricAttributes.Add(TEXT("res.y"), FString::FromInt(GSystemResolution.ResY));

	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (World)
	{
		MetricAttributes.Add(TEXT("map"), World->GetMapName());
	}
}
