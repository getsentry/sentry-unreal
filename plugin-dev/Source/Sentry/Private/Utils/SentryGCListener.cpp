// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryGCListener.h"

#include "SentryDefines.h"
#include "SentrySubsystem.h"
#include "SentryUnit.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "UObject/UObjectGlobals.h"
#include "UnrealEngine.h"

FSentryGCListener::FSentryGCListener()
	: GCStartTime(0.0)
{
	CacheAttributes();

	GCStartedHandle = FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().AddRaw(this, &FSentryGCListener::OnGCStarted);
	PostGCHandle = FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FSentryGCListener::OnPostGC);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSentryGCListener::OnMapLoaded);

	UE_LOG(LogSentrySdk, Log, TEXT("GC listener started."));
}

FSentryGCListener::~FSentryGCListener()
{
	if (GCStartedHandle.IsValid())
	{
		FCoreUObjectDelegates::GetGarbageCollectStartedDelegate().Remove(GCStartedHandle);
		GCStartedHandle.Reset();
	}

	if (PostGCHandle.IsValid())
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(PostGCHandle);
		PostGCHandle.Reset();
	}

	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
}

void FSentryGCListener::OnGCStarted()
{
	GCStartTime = FPlatformTime::Seconds();
}

void FSentryGCListener::OnPostGC()
{
	if (GCStartTime <= 0.0)
	{
		return;
	}

	const float PauseMs = static_cast<float>((FPlatformTime::Seconds() - GCStartTime) * 1000.0);

	USentrySubsystem* Sentry = GEngine ? GEngine->GetEngineSubsystem<USentrySubsystem>() : nullptr;
	if (!Sentry || !Sentry->IsEnabled())
	{
		return;
	}

	Sentry->AddDistributionWithAttributes(TEXT("game.perf.gc_time"), PauseMs, FSentryUnit(ESentryUnit::Millisecond), MetricAttributes);
}

void FSentryGCListener::OnMapLoaded(UWorld* World)
{
	if (World)
	{
		MetricAttributes.Add(TEXT("map"), World->GetMapName());
	}
}

void FSentryGCListener::CacheAttributes()
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
