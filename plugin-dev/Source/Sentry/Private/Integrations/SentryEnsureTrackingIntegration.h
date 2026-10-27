// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

class ISentrySubsystem;

/**
 * Captures failed ensures as non-fatal events.
 */
class FSentryEnsureTrackingIntegration final : public ISentryIntegration
{
public:
	explicit FSentryEnsureTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);

	virtual const TCHAR* GetName() const override { return TEXT("EnsureTracking"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	TSharedRef<ISentrySubsystem> NativeImpl;

	FDelegateHandle EnsureHandle;
};
