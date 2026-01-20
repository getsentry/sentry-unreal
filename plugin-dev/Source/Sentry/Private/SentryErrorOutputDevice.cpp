// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryErrorOutputDevice.h"

#include "HAL/PlatformAtomics.h"
#include "Misc/AssertionMacros.h"

FSentryErrorOutputDevice::FSentryErrorOutputDevice(FOutputDeviceError* Parent)
	: ParentDevice(Parent)
{
}

void FSentryErrorOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (FDebug::HasAsserted())
	{
		static int32 CallCount = 0;
		int32 NewCallCount = FPlatformAtomics::InterlockedIncrement(&CallCount);
		if (NewCallCount == 1)
		{
			OnAssert.Broadcast(V);
		}
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
