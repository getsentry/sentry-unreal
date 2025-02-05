#pragma once

#if USE_SENTRY_NATIVE

#include "Microsoft/MicrosoftSentrySubsystem.h"

class FXboxSentrySubsystem : public FMicrosoftSentrySubsystem
{
public:
	virtual void ConfigureHandlerPath(sentry_options_t* Options) override {}
};

typedef FXboxSentrySubsystem FPlatformSentrySubsystem;

#endif // USE_SENTRY_NATIVE
