#pragma once

#if USE_SENTRY_NATIVE

#include "Misc/EngineVersionComparison.h"

#include "GenericPlatform/GenericPlatformSentrySubsystem.h"

class FMicrosoftSentrySubsystem : public FGenericPlatformSentrySubsystem
{
protected:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override;
    virtual void ConfigureDatabasePath(sentry_options_t* Options) override;
    virtual void ConfigureLogFileAttachment(sentry_options_t* Options) override;
    virtual void ConfigureScreenshotAttachment(sentry_options_t* Options) override;

#if !UE_VERSION_OLDER_THAN(5, 3, 0)
    virtual int32 GetEnsureFramesToSkip() const override { return 8; }
#endif // !UE_VERSION_OLDER_THAN(5, 3, 0)
	virtual int32 GetAssertionFramesToSkip() const override { return 7; }
};

#endif // USE_SENTRY_NATIVE
