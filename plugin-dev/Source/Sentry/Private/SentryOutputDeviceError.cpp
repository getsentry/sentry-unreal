// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryOutputDeviceError.h"

#include "Misc/AssertionMacros.h"

FSentryOutputDeviceError::FSentryOutputDeviceError(FOutputDeviceError* Parent)
	: ParentDevice(Parent)
{
}

void FSentryOutputDeviceError::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if(FDebug::HasAsserted())
	{
		OnAssert.Broadcast(V);
	}

	if (!ParentDevice)
		return;

	ParentDevice->Serialize(V, Verbosity, Category);
}

void FSentryOutputDeviceError::HandleError()
{
	if (!ParentDevice)
		return;

	ParentDevice->HandleError();
}

FOutputDeviceError* FSentryOutputDeviceError::GetParentDevice()
{
	return ParentDevice;
}
