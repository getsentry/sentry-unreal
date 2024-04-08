// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryOutputDeviceError.h"

FSentryOutputDeviceError::FSentryOutputDeviceError(FOutputDeviceError* Parent)
	: ParentDevice(Parent)
{
}

void FSentryOutputDeviceError::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (ParentDevice)
	{
		ParentDevice->Serialize(V, Verbosity, Category);
	}
}

void FSentryOutputDeviceError::HandleError()
{
	if (ParentDevice)
	{
		ParentDevice->HandleError();
	}
}

FOutputDeviceError* FSentryOutputDeviceError::GetParentDevice()
{
	return ParentDevice;
}
