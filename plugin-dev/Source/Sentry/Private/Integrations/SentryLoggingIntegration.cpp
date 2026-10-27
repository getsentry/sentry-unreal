// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryLoggingIntegration.h"

#include "SentryOutputDevice.h"

#include "Interface/SentrySubsystemInterface.h"

#include "CoreGlobals.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/OutputDeviceRedirector.h"

FSentryLoggingIntegration::FSentryLoggingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

FSentryLoggingIntegration::~FSentryLoggingIntegration() = default;

bool FSentryLoggingIntegration::Setup(const USentrySettings& Settings)
{
	if (!GLog)
	{
		return false;
	}

	OutputDevice = MakeUnique<FSentryOutputDevice>(NativeImpl, Settings);

	GLog->AddOutputDevice(OutputDevice.Get());
#if UE_VERSION_OLDER_THAN(5, 7, 0)
	GLog->SerializeBacklog(OutputDevice.Get());
#endif

	return true;
}

void FSentryLoggingIntegration::Teardown()
{
	if (GLog && OutputDevice)
	{
		GLog->RemoveOutputDevice(OutputDevice.Get());
	}

	OutputDevice.Reset();
}
