// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Utils/SentryPerformanceMetricAttributes.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "UnrealEngine.h"

FSentryPerformanceMetricAttributes::FSentryPerformanceMetricAttributes()
{
	CacheAttributes();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSentryPerformanceMetricAttributes::OnMapLoaded);
}

FSentryPerformanceMetricAttributes::~FSentryPerformanceMetricAttributes()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
}

const TMap<FString, FSentryVariant>& FSentryPerformanceMetricAttributes::GetAttributes() const
{
	return Attributes;
}

void FSentryPerformanceMetricAttributes::OnMapLoaded(UWorld* World)
{
	if (World)
	{
		Attributes.Add(TEXT("map"), World->GetMapName());
	}
}

void FSentryPerformanceMetricAttributes::CacheAttributes()
{
	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());
	if (GpuDriverInfo.IsValid())
	{
		Attributes.Add(TEXT("gpu.name"), GpuDriverInfo.DeviceDescription);
	}

	Attributes.Add(TEXT("cpu.cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	Attributes.Add(TEXT("ram.gb"), FString::FromInt(FPlatformMemory::GetConstants().TotalPhysicalGB));
	Attributes.Add(TEXT("res.x"), FString::FromInt(GSystemResolution.ResX));
	Attributes.Add(TEXT("res.y"), FString::FromInt(GSystemResolution.ResY));

	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	if (World)
	{
		Attributes.Add(TEXT("map"), World->GetMapName());
	}
}
