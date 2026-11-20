// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "OutputDevice/SentryErrorOutputDevice.h"

#include "HAL/PlatformAtomics.h"
#include "Misc/AssertionMacros.h"

FSentryErrorOutputDevice::FSentryErrorOutputDevice(FOutputDeviceError* Parent)
	: ParentDevice(Parent)
	, AssertCount(0)
{
}

void FSentryErrorOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (HasAsserted() && FPlatformAtomics::InterlockedIncrement(&AssertCount) == 1)
	{
		OnAssert.Broadcast(V);
	}

	if (!ParentDevice)
		return;

	ParentDevice->Serialize(V, Verbosity, Category);
}

void FSentryErrorOutputDevice::HandleError()
{
	if (!ParentDevice)
		return;

	ParentDevice->HandleError();
}

FOutputDeviceError* FSentryErrorOutputDevice::GetParentDevice()
{
	return ParentDevice;
}

bool FSentryErrorOutputDevice::HasAsserted() const
{
	return FDebug::HasAsserted();
}
