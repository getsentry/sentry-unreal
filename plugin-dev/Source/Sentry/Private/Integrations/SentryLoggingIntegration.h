// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "Integrations/SentryIntegration.h"

class ISentrySubsystem;
class FSentryOutputDevice;

/**
 * Forwards engine log output to Sentry as structured logs and/or breadcrumbs, depending on plugin settings.
 */
class FSentryLoggingIntegration final : public ISentryIntegration
{
public:
	explicit FSentryLoggingIntegration(const TSharedRef<ISentrySubsystem>& InNativeImpl);
	virtual ~FSentryLoggingIntegration() override;

	virtual const TCHAR* GetName() const override { return TEXT("Logging"); }

	virtual bool Setup(const USentrySettings& Settings) override;
	virtual void Teardown() override;

private:
	TSharedRef<ISentrySubsystem> NativeImpl;

	TUniquePtr<FSentryOutputDevice> OutputDevice;
};
