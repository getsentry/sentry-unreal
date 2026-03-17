// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryFrameTracker.h"

#include "SentryDefines.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CoreDelegates.h"
#include "UnrealEngine.h"

extern ENGINE_API float GAverageFPS;

FSentryFrameTracker::FSentryFrameTracker(int32 InSampleInterval)
	: SampleInterval(FMath::Max(InSampleInterval, 1))
	, FrameCount(0)
	, FrameStartTime(0.0)
{
	CacheAttributes();
}

FSentryFrameTracker::~FSentryFrameTracker()
{
	Stop();
}

void FSentryFrameTracker::Start()
{
	if (BeginFrameHandle.IsValid())
	{
		return;
	}

	BeginFrameHandle = FCoreDelegates::OnBeginFrame.AddRaw(this, &FSentryFrameTracker::OnBeginFrame);
	EndFrameHandle = FCoreDelegates::OnEndFrame.AddRaw(this, &FSentryFrameTracker::OnEndFrame);

	UE_LOG(LogSentrySdk, Log, TEXT("Frame tracker started (sample interval: %d frames)."), SampleInterval);
}

void FSentryFrameTracker::Stop()
{
	if (BeginFrameHandle.IsValid())
	{
		FCoreDelegates::OnBeginFrame.Remove(BeginFrameHandle);
		BeginFrameHandle.Reset();
	}

	if (EndFrameHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(EndFrameHandle);
		EndFrameHandle.Reset();
	}
}

void FSentryFrameTracker::OnBeginFrame()
{
	FrameStartTime = FPlatformTime::Seconds();
}

void FSentryFrameTracker::OnEndFrame()
{
	if (FrameStartTime <= 0.0)
	{
		return;
	}

	++FrameCount;

	if (FrameCount % SampleInterval != 0)
	{
		return;
	}

	const float FrameTimeMs = static_cast<float>((FPlatformTime::Seconds() - FrameStartTime) * 1000.0);

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return;
	}

	Sentry->AddDistributionWithAttributes(TEXT("game.frame.duration"), FrameTimeMs, FSentryUnit(ESentryUnit::Millisecond), MetricAttributes);
	Sentry->AddGaugeWithAttributes(TEXT("game.fps"), GAverageFPS, FSentryUnit(ESentryUnit::None), MetricAttributes);
}

void FSentryFrameTracker::CacheAttributes()
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
}
