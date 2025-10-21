// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

#include "Utils/SentryProtonUtils.h"

void FWindowsSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	// Check if running under Wine/Proton - Crashpad handler may not work correctly
	FSentryProtonUtils::FProtonInfo ProtonInfo = FSentryProtonUtils::DetectProtonEnvironment();

	if (ProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Running under Wine/Proton - Crashpad handler (crashpad_handler.exe) may not function correctly due to incomplete Windows exception handling API emulation."));
		UE_LOG(LogSentrySdk, Warning, TEXT("Crash reports may be incomplete or missing. Consider using the in-process backend instead."));
		// Don't set handler path - it likely won't work properly under Wine
		// The GenericPlatformSentrySubsystem will configure the in-process backend instead
		return;
	}

	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad executable couldn't be found at: %s"), *HandlerPath);
		return;
	}

	UE_LOG(LogSentrySdk, Verbose, TEXT("Configuring Crashpad handler at: %s"), *HandlerPath);
	sentry_options_set_handler_pathw(Options, *HandlerPath);
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
