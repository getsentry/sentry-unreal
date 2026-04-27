// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Performance/SentryPerfMetricAttributes.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "UnrealEngine.h"

FSentryPerfMetricAttributes::FSentryPerfMetricAttributes()
{
	CacheAttributes();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FSentryPerfMetricAttributes::OnMapLoaded);
}

FSentryPerfMetricAttributes::~FSentryPerfMetricAttributes()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
}

const TMap<FString, FSentryVariant>& FSentryPerfMetricAttributes::GetAttributes() const
{
	return Attributes;
}

void FSentryPerfMetricAttributes::OnMapLoaded(UWorld* World)
{
	if (World)
	{
		Attributes.Add(TEXT("map"), World->GetMapName());
	}
}

void FSentryPerfMetricAttributes::CacheAttributes()
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
