// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentryOutputDeviceError.h"

#include "SentryDefines.h"

FSentryOutputDeviceError::FSentryOutputDeviceError(FOutputDeviceError* Parent)
	: ParentDevice(Parent)
{
}

void FSentryOutputDeviceError::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	static int32 CallCount = 0;
	int32 NewCallCount = FPlatformAtomics::InterlockedIncrement(&CallCount);
	if(GIsCriticalError == 0 && NewCallCount == 1)
	{
		GIsCriticalError = 1;

		UE_LOG(LogWindows, Error, TEXT("Error called: %s"), V);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Error reentered: %s"), V);
	}

	if (GIsGuarded)
	{
		OnError.Broadcast(V);
	}
	else
	{
		HandleError();
		FPlatformMisc::RequestExit( true);
	}
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
