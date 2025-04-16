#pragma once

#if USE_SENTRY_NATIVE

#include "Misc/EngineVersionComparison.h"

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FMicrosoftSentrySubsystem : public FGenericPlatformSentrySubsystem
{	
public:
	virtual void InitWithSettings(
		const USentrySettings* Settings,
		USentryBeforeSendHandler* BeforeSendHandler,
		USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
		USentryTraceSampler* TraceSampler
	) override;

protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;
	virtual void ConfigureScreenshotAttachment(sentry_options_t* Options) override;
};

#endif // USE_SENTRY_NATIVE
