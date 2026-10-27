// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

class ISentrySubsystem;
class FSentryErrorOutputDevice;

/**
 * Handles failed assertions (check, verify, etc.) by wrapping the engine's error output device
 * so the platform SDK can prepare the crash report before the application terminates.
 */
class FSentryAssertTrackingIntegration final : public ISentryIntegration
{
public:
	explicit FSentryAssertTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);
	virtual ~FSentryAssertTrackingIntegration() override;

	virtual const TCHAR* GetName() const override { return TEXT("AssertTracking"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	TSharedRef<ISentrySubsystem> NativeImpl;

	TUniquePtr<FSentryErrorOutputDevice> ErrorOutputDevice;
	FDelegateHandle AssertHandle;
};
