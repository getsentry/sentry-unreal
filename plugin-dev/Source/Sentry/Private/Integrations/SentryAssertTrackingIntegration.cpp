// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryAssertTrackingIntegration.h"

#include "SentryErrorOutputDevice.h"

#include "Interface/SentrySubsystemInterface.h"

#include "CoreGlobals.h"

FSentryAssertTrackingIntegration::FSentryAssertTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

FSentryAssertTrackingIntegration::~FSentryAssertTrackingIntegration() = default;

bool FSentryAssertTrackingIntegration::Setup(const USentrySettings& Settings)
{
	if (!GError)
	{
		return false;
	}

	ErrorOutputDevice = MakeUnique<FSentryErrorOutputDevice>(GError);
	ErrorOutputDevice->OnAssert.AddRaw(this, &FSentryAssertTrackingIntegration::OnAssert);

	GError = ErrorOutputDevice.Get();

	return true;
}

void FSentryAssertTrackingIntegration::Teardown()
{
	if (GError && ErrorOutputDevice)
	{
		ErrorOutputDevice->OnAssert.RemoveAll(this);

		GError = ErrorOutputDevice->GetParentDevice();
		ErrorOutputDevice.Reset();
	}
}

void FSentryAssertTrackingIntegration::OnAssert(const FString& Message)
{
	// Called from the thread that hit the assertion
	NativeImpl->HandleAssert();
}
