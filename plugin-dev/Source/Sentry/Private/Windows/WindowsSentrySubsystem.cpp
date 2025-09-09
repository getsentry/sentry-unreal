// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

void FWindowsSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad executable couldn't be found."));
		return;
	}

	sentry_options_set_handler_pathw(Options, *HandlerPath);
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
