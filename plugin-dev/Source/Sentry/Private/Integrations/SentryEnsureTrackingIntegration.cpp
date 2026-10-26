// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryEnsureTrackingIntegration.h"

#include "Interface/SentrySubsystemInterface.h"

#include "CoreGlobals.h"
#include "Misc/CoreDelegates.h"

FSentryEnsureTrackingIntegration::FSentryEnsureTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

bool FSentryEnsureTrackingIntegration::Setup(const USentrySettings& Settings)
{
	// Called from the thread that hit the ensure, which may still be running after teardown.
	// Capture the native implementation by value so the handler doesn't depend on this integration's lifetime.
	EnsureHandle = FCoreDelegates::OnHandleSystemEnsure.AddLambda([NativeImpl = NativeImpl]()
	{
		FString EnsureMessage = GErrorHist;
		NativeImpl->CaptureEnsure(TEXT("Ensure failed"), EnsureMessage.TrimStartAndEnd());
	});

	return true;
}

void FSentryEnsureTrackingIntegration::Teardown()
{
	if (EnsureHandle.IsValid())
	{
		FCoreDelegates::OnHandleSystemEnsure.Remove(EnsureHandle);
		EnsureHandle.Reset();
	}
}
