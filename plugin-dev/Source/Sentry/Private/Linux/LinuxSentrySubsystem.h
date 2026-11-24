// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FLinuxSentrySubsystem : public FGenericPlatformSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers) override;

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureCertsPath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;
	virtual void ConfigureCrashReporterPath(sentry_options_t* Options) override;

	virtual FString GetHandlerExecutableName() const override;
	virtual FString GetCrashReporterExecutableName() const override { return TEXT("Sentry.CrashReporter"); }

	virtual bool IsScreenshotSupported() const override { return true; }
	virtual bool IsHangTrackingSupported() const override { return true; }

	virtual FString GetDeviceType() const override;
};

typedef FLinuxSentrySubsystem FPlatformSentrySubsystem;

#endif // USE_SENTRY_NATIVE
