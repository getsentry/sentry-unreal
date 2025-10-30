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

		// Set Runtime context (Wine/Proton)
		TMap<FString, FSentryVariant> RuntimeContext;
		RuntimeContext.Add(TEXT("name"), FSentryPlatformDetectionUtils::GetRuntimeName(WineProtonInfo));
		RuntimeContext.Add(TEXT("version"), FSentryPlatformDetectionUtils::GetRuntimeVersion(WineProtonInfo));
		SetContext(TEXT("runtime"), RuntimeContext);

		// Set OS context (Linux distro)
		if (!DistroInfo.ID.IsEmpty())
		{
			TMap<FString, FSentryVariant> OSContext;
			OSContext.Add(TEXT("name"), FSentryPlatformDetectionUtils::GetOSNameForContext(DistroInfo));
			if (!DistroInfo.Version.IsEmpty())
			{
				OSContext.Add(TEXT("version"), DistroInfo.Version);
			}
			if (!DistroInfo.ID.IsEmpty())
			{
				OSContext.Add(TEXT("kernel_version"), DistroInfo.ID);
			}
			SetContext(TEXT("os"), OSContext);
		}

		// Set Device context (Handheld device)
		if (HandheldInfo.bIsHandheld)
		{
			TMap<FString, FSentryVariant> DeviceContext;
			if (!HandheldInfo.Manufacturer.IsEmpty())
			{
				DeviceContext.Add(TEXT("manufacturer"), HandheldInfo.Manufacturer);
			}
			if (!HandheldInfo.Model.IsEmpty())
			{
				DeviceContext.Add(TEXT("model"), HandheldInfo.Model);
			}
			if (!HandheldInfo.Codename.IsEmpty())
			{
				DeviceContext.Add(TEXT("name"), HandheldInfo.Codename);
			}
			SetContext(TEXT("device"), DeviceContext);
		}

		// Set platform tags
		SetTag(TEXT("wine_proton"), WineProtonInfo.bIsProton ? TEXT("proton") : TEXT("wine"));
		if (!WineProtonInfo.Version.IsEmpty())
		{
			SetTag(TEXT("wine_version"), WineProtonInfo.Version);
		}
		if (WineProtonInfo.bIsProton && !WineProtonInfo.ProtonBuildName.IsEmpty())
		{
			SetTag(TEXT("proton_build"), WineProtonInfo.ProtonBuildName);
		}
		if (WineProtonInfo.bIsExperimental)
		{
			SetTag(TEXT("proton_experimental"), TEXT("true"));
		}
		if (!DistroInfo.ID.IsEmpty())
		{
			SetTag(TEXT("linux_distro"), DistroInfo.ID);
		}
		if (DistroInfo.bIsSteamOS)
		{
			SetTag(TEXT("steamos"), TEXT("true"));
		}
		if (DistroInfo.bIsBazzite)
		{
			SetTag(TEXT("bazzite"), TEXT("true"));
		}
		if (DistroInfo.bIsGamingDistro)
		{
			SetTag(TEXT("gaming_distro"), TEXT("true"));
		}
		if (HandheldInfo.bIsSteamDeck)
		{
			SetTag(TEXT("steam_deck"), TEXT("true"));
			if (HandheldInfo.bIsSteamDeckOLED)
			{
				SetTag(TEXT("steam_deck_oled"), TEXT("true"));
			}
		}
		if (HandheldInfo.bIsHandheld && !HandheldInfo.bIsSteamDeck)
		{
			SetTag(TEXT("handheld"), TEXT("true"));
		}
		if (FSentryPlatformDetectionUtils::IsRunningSteam())
		{
			SetTag(TEXT("running_steam"), TEXT("true"));
		}
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
