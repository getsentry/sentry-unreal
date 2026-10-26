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
	FCoreDelegates::OnHandleSystemEnsure.AddRaw(this, &FSentryEnsureTrackingIntegration::OnEnsure);

	return true;
}

void FSentryEnsureTrackingIntegration::Teardown()
{
	FCoreDelegates::OnHandleSystemEnsure.RemoveAll(this);
}

void FSentryEnsureTrackingIntegration::OnEnsure()
{
	// Called from the thread that hit the ensure
	FString EnsureMessage = GErrorHist;
	NativeImpl->CaptureEnsure(TEXT("Ensure failed"), EnsureMessage.TrimStartAndEnd());
}
