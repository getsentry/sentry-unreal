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

	// Called from the thread that hit the assertion, which may still be running after teardown.
	// Capture the native implementation by value so the handler doesn't depend on this integration's lifetime.
	AssertHandle = ErrorOutputDevice->OnAssert.AddLambda([NativeImpl = NativeImpl](const FString& Message)
	{
		NativeImpl->HandleAssert();
	});

	GError = ErrorOutputDevice.Get();

	return true;
}

void FSentryAssertTrackingIntegration::Teardown()
{
	if (GError && ErrorOutputDevice)
	{
		ErrorOutputDevice->OnAssert.Remove(AssertHandle);
		AssertHandle.Reset();

		GError = ErrorOutputDevice->GetParentDevice();
		ErrorOutputDevice.Reset();
	}
}
