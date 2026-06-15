// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE && !SENTRY_WINGDK

#include "Utils/SentryPlatformInfo.h"

#include "Microsoft/MicrosoftSentrySubsystem.h"

class FWindowsSentrySubsystem : public FMicrosoftSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* Settings, const FSentryCallbackHandlers& CallbackHandlers) override;

	virtual bool IsHangTrackingSupported() const override { return true; }
	virtual FString GetDeviceType() const override;

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
	virtual void ConfigureStackCaptureStrategy(sentry_options_t* Options) override;
	virtual void ConfigureCrashReporterPath(sentry_options_t* Options) override;
	virtual void ConfigureScreenshotCapturing(sentry_options_t* Options) override;

	virtual FString GetHandlerExecutableName() const override;
	virtual FString GetCrashReporterExecutableName() const override { return TEXT("Sentry.CrashReporter.exe"); }

	virtual bool IsScreenshotSupported() const override { return true; }
	virtual bool IsOutOfProcessScreenshotEnabled() const override { return bOutOfProcessScreenshots; }

private:
	/** Wine/Proton detection info */
	FWineProtonInfo WineProtonInfo;

	/** Whether native out-of-process screenshot capturing is enabled */
	bool bOutOfProcessScreenshots = false;
};

typedef FWindowsSentrySubsystem FPlatformSentrySubsystem;

#endif // USE_SENTRY_NATIVE && !SENTRY_WINGDK
