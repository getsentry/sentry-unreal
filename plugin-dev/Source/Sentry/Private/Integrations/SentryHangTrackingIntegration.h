// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

class ISentrySubsystem;
class FSentryHangWatcher;

/**
 * Captures thread hangs detected via the engine's FThreadHeartBeat on platforms
 * where the underlying platform SDK doesn't track hangs natively.
 */
class FSentryHangTrackingIntegration final : public ISentryIntegration
{
public:
	explicit FSentryHangTrackingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);

	virtual const TCHAR* GetName() const override { return TEXT("HangTracking"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	TSharedRef<ISentrySubsystem> NativeImpl;

	TSharedPtr<FSentryHangWatcher> HangWatcher;
};
