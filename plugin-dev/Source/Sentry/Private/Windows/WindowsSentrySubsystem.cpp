// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"
#include "Utils/SentryPlatformDetectionUtils.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <winternl.h>

void FWindowsSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryBeforeLogHandler* BeforeLogHandler, USentryTraceSampler* TraceSampler)
{
	// Detect Wine/Proton before initializing
	WineProtonInfo = FSentryPlatformDetectionUtils::DetectWineProton();

	// Call parent implementation
	FMicrosoftSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, BeforeLogHandler, TraceSampler);

	// Add Wine/Proton context for all events if detected
	if (WineProtonInfo.bIsRunningUnderWine && IsEnabled())
	{	
		// Detect Linux distro if running under Wine/Proton
		DistroInfo = FSentryPlatformDetectionUtils::DetectLinuxDistro();
		HandheldInfo = FSentryPlatformDetectionUtils::DetectHandheldDevice();

		// Use centralized platform detection utilities to set contexts
		FSentryPlatformDetectionUtils::SetSentryRuntimeContext(WineProtonInfo);
		FSentryPlatformDetectionUtils::SetSentryOSContext(DistroInfo);
		FSentryPlatformDetectionUtils::SetSentryDeviceContext(HandheldInfo);
		FSentryPlatformDetectionUtils::SetSentryPlatformTags(&WineProtonInfo, &DistroInfo, &HandheldInfo);
	}
}

void FWindowsSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crashpad executable couldn't be found."));
		return;
	}

	sentry_options_set_handler_pathw(Options, *HandlerPath);

	// Enable stack capture adjustment for Wine/Proton
	if (WineProtonInfo.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Enabling Crashpad stack capture adjustment for Wine/Proton compatibility"));
		sentry_options_set_crashpad_limit_stack_capture_to_sp(Options, 1);
	}
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	// Context and tags are already set globally during InitWithSettings
	// Just call parent implementation
	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
