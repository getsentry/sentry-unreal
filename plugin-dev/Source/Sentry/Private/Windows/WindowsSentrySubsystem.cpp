// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE && !SENTRY_WINGDK

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Utils/SentryPlatformDetectionUtils.h"

#include "Misc/Paths.h"

void FWindowsSentrySubsystem::InitWithSettings(const USentrySettings* Settings, const FSentryCallbackHandlers& CallbackHandlers)
{
	bOutOfProcessScreenshots = Settings->EnableOutOfProcessScreenshots;

	bIsRunningUnderWine = FSentryPlatformDetectionUtils::IsRunningUnderWine();

	// Call parent implementation (handles crash logger initialization)
	FMicrosoftSentrySubsystem::InitWithSettings(Settings, CallbackHandlers);

	if (Settings->EnableExternalCrashReporter)
	{
		ConfigureCrashReporterAppearance(Settings);
	}

	if (bIsRunningUnderWine && IsEnabled())
	{
		// Override OS context when running under Wine/Proton (always show Linux host, not Windows)
		if (FSentryPlatformDetectionUtils::IsSteamOS())
		{
			SetContext(TEXT("os"), FSentryPlatformDetectionUtils::MakeOsContext(TEXT("SteamOS"), true));
			SetTag(TEXT("steamos"), TEXT("true"));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: SteamOS (detected via Wine/Proton)"));
		}
		else if (FSentryPlatformDetectionUtils::IsBazzite())
		{
			SetContext(TEXT("os"), FSentryPlatformDetectionUtils::MakeOsContext(TEXT("Bazzite"), true));
			SetTag(TEXT("bazzite"), TEXT("true"));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: Bazzite (detected via Wine/Proton)"));
		}
		else
		{
			SetContext(TEXT("os"), FSentryPlatformDetectionUtils::MakeOsContext(TEXT("Linux"), false));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: Linux (detected via Wine/Proton)"));
		}

		if (FSentryPlatformDetectionUtils::IsRunningSteam())
		{
			SetTag(TEXT("steam"), TEXT("true"));
		}
	}
}

FString FWindowsSentrySubsystem::GetHandlerExecutableName() const
{
	return bUseNativeBackend ? TEXT("sentry-crash.exe") : TEXT("crashpad_handler.exe");
}

void FWindowsSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crash handler executable couldn't be found at: %s"), *HandlerPath);
		return;
	}

	sentry_options_set_handler_pathw(Options, *HandlerPath);
}

void FWindowsSentrySubsystem::ConfigureStackCaptureStrategy(sentry_options_t* Options)
{
	if (bIsRunningUnderWine && !bUseNativeBackend)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Enabling Crashpad stack capture adjustment for Wine/Proton compatibility"));
		sentry_options_set_crashpad_limit_stack_capture_to_sp(Options, 1);
	}
}

void FWindowsSentrySubsystem::ConfigureCrashReporterPath(sentry_options_t* Options)
{
	const FString CrashReporterPath = GetCrashReporterPath();
	if (!FPaths::FileExists(CrashReporterPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("External crash reporter executable couldn't be found at: %s"), *CrashReporterPath);
		return;
	}
	sentry_options_set_external_crash_reporter_pathw(Options, *CrashReporterPath);
}

void FWindowsSentrySubsystem::ConfigureScreenshotCapturing(sentry_options_t* Options)
{
	if (bOutOfProcessScreenshots)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Native out-of-process screenshot capturing enabled"));
		sentry_options_set_attach_screenshot(Options, 1);
	}
}

FString FWindowsSentrySubsystem::GetDeviceType() const
{
	if (FSentryPlatformDetectionUtils::IsSteamDeck())
	{
		return TEXT("Handheld");
	}

	return FMicrosoftSentrySubsystem::GetDeviceType();
}

void FWindowsSentrySubsystem::ConfigureAppHangTracking()
{
	FGenericPlatformSentrySubsystem::ConfigureAppHangTracking();
}

void FWindowsSentrySubsystem::ResetAppHangTracking()
{
	FGenericPlatformSentrySubsystem::ResetAppHangTracking();
}

bool FWindowsSentrySubsystem::IsAppHangTrackingActive() const
{
	return FGenericPlatformSentrySubsystem::IsAppHangTrackingActive();
}

#endif // USE_SENTRY_NATIVE && !SENTRY_WINGDK
