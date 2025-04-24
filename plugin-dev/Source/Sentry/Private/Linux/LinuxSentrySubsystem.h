#pragma once

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FLinuxSentrySubsystem : public FGenericPlatformSentrySubsystem
{
protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
	virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
	virtual void ConfigureCertsPath(sentry_options_t* Options) override;
	virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;
	virtual void ConfigureScreenshotAttachment(sentry_options_t* Options) override;
	virtual void ConfigureGpuDumpAttachment(sentry_options_t* Options) override;

	virtual FString GetHandlerExecutableName() const override { return TEXT("crashpad_handler"); }
};

typedef FLinuxSentrySubsystem FPlatformSentrySubsystem;

#endif // USE_SENTRY_NATIVE
