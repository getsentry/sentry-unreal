// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "Integrations/SentryHangTrackingIntegration.h"

#include "SentrySettings.h"

#include "HangTracking/SentryHangWatcher.h"
#include "Interface/SentrySubsystemInterface.h"

FSentryHangTrackingIntegration::FSentryHangTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl)
	: NativeImpl(InNativeImpl)
{
}

bool FSentryHangTrackingIntegration::Setup(const USentrySettings& Settings)
{
	if (!Settings.EnableHangTracking || !NativeImpl->IsHangTrackingSupported() || NativeImpl->IsNativeHangTrackingEnabled())
	{
		return false;
	}

	HangWatcher = MakeShared<FSentryHangWatcher>(Settings.HangTimeoutDuration);

	// Called from the watcher thread
	HangWatcher->OnHangDetected.BindLambda([NativeImpl = NativeImpl](uint32 HungThreadId, double HangDuration)
	{
		NativeImpl->CaptureHang(HungThreadId);
	});

	return HangWatcher->Start();
}

void FSentryHangTrackingIntegration::Teardown()
{
	if (HangWatcher.IsValid())
	{
		// Joins the watcher thread so no hangs can be reported afterwards
		HangWatcher->Stop();
		HangWatcher.Reset();
	}
}
