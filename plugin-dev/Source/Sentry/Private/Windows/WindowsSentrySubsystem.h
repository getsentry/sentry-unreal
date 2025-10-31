// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if USE_SENTRY_NATIVE

#include "Microsoft/MicrosoftSentrySubsystem.h"
#include "Utils/SentryPlatformInfo.h"

class FWindowsSentrySubsystem : public FMicrosoftSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryBeforeLogHandler* BeforeLogHandler, USentryTraceSampler* TraceSampler) override;

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;

	virtual FString GetHandlerExecutableName() const override { return TEXT("crashpad_handler.exe"); }

	virtual sentry_value_t OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure) override;

private:
	/** Wine/Proton detection info */
	FWineProtonInfo WineProtonInfo;
};

typedef FWindowsSentrySubsystem FPlatformSentrySubsystem;

#endif // USE_SENTRY_NATIVE
