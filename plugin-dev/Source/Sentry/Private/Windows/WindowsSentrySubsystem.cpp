// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE && !SENTRY_WINGDK

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Utils/SentryPlatformDetectionUtils.h"

#include "HAL/FileManager.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

#if USE_SENTRY_CRASH_VIDEO
#include "CrashVideo/SentryCrashVideoSubsystem.h"
#endif

FWindowsSentrySubsystem::FWindowsSentrySubsystem() = default;
FWindowsSentrySubsystem::~FWindowsSentrySubsystem() = default;

void FWindowsSentrySubsystem::InitWithSettings(const USentrySettings* Settings, const FSentryCallbackHandlers& CallbackHandlers)
{
	bOutOfProcessScreenshots = Settings->EnableOutOfProcessScreenshots;

	// Detect Wine/Proton before initializing
	WineProtonInfo = FSentryPlatformDetectionUtils::DetectWineProton();

	// Call parent implementation (handles crash logger initialization)
	FMicrosoftSentrySubsystem::InitWithSettings(Settings, CallbackHandlers);

	if (Settings->EnableExternalCrashReporter)
	{
		ConfigureCrashReporterAppearance(Settings);
	}

	// Add Wine/Proton context for all events if detected
	if (WineProtonInfo.bIsRunningUnderWine && IsEnabled())
	{
		// Set Runtime context (Wine/Proton)
		TMap<FString, FSentryVariant> RuntimeContext;
		RuntimeContext.Add(TEXT("name"), FSentryPlatformDetectionUtils::GetRuntimeName(WineProtonInfo));
		RuntimeContext.Add(TEXT("version"), FSentryPlatformDetectionUtils::GetRuntimeVersion(WineProtonInfo));
		SetContext(TEXT("runtime"), RuntimeContext);

		// Override OS context when running under Wine/Proton (always show Linux, not Windows)
		TMap<FString, FSentryVariant> OSContext;
		OSContext.Add(TEXT("type"), TEXT("os")); // Explicitly set context type

		// Detect specific Linux distros
		if (FSentryPlatformDetectionUtils::IsSteamOS())
		{
			OSContext.Add(TEXT("name"), TEXT("SteamOS"));
			SetTag(TEXT("steamos"), TEXT("true"));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: SteamOS (detected via Wine/Proton)"));
		}
		else if (FSentryPlatformDetectionUtils::IsBazzite())
		{
			OSContext.Add(TEXT("name"), TEXT("Bazzite"));
			SetTag(TEXT("bazzite"), TEXT("true"));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: Bazzite (detected via Wine/Proton)"));
		}
		else
		{
			// Default to "Linux" for unknown distros
			OSContext.Add(TEXT("name"), TEXT("Linux"));
			UE_LOG(LogSentrySdk, Log, TEXT("Overriding OS context: Linux (detected via Wine/Proton)"));
		}

		SetContext(TEXT("os"), OSContext);

		// Set platform tags
		SetTag(TEXT("compatibility"), WineProtonInfo.bIsProton ? TEXT("proton") : TEXT("wine"));
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
	if (WineProtonInfo.bIsRunningUnderWine && !bUseNativeBackend)
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

void FWindowsSentrySubsystem::ConfigureSessionReplayCapturing(sentry_options_t* Options)
{
#if USE_SENTRY_CRASH_VIDEO
	// Clear replay videos captured during previous session if any.
	IFileManager::Get().DeleteDirectory(*FPaths::Combine(GetDatabasePath(), TEXT("replays")), false, true);

	const USentrySettings* Settings = GetDefault<USentrySettings>();
	CrashVideo = MakeUnique<FSentryCrashVideoSubsystem>();
	if (!CrashVideo->Initialize(Settings, GetReplayPath()))
	{
		CrashVideo.Reset();
	}
#endif
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
#if USE_SENTRY_CRASH_VIDEO
	if (CrashVideo && CrashVideo->HasSnapshotOnDisk())
	{
		// Register the rolling video file as a crash attachment. Sentry-native
		// forwards this to crashpad's client->AddAttachment IPC; the handler
		// reads the file off disk when it serialises the report.
		sentry_attach_filew(*CrashVideo->GetAttachmentPath());
	}
#endif

	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

void FWindowsSentrySubsystem::Close()
{
#if USE_SENTRY_CRASH_VIDEO
	if (CrashVideo)
	{
		CrashVideo->Shutdown();
		CrashVideo.Reset();
	}
#endif

	FMicrosoftSentrySubsystem::Close();
}

FString FWindowsSentrySubsystem::GetDeviceType() const
{
	if (FSentryPlatformDetectionUtils::IsSteamDeck())
	{
		return TEXT("Handheld");
	}

	return FMicrosoftSentrySubsystem::GetDeviceType();
}

#if USE_SENTRY_CRASH_VIDEO
FString FWindowsSentrySubsystem::GetReplayPath() const
{
	const FString ReplayPath = FPaths::Combine(GetDatabasePath(), TEXT("replays"),
		FString::Printf(TEXT("replay-%s.mp4"), *FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower)));
	return FPaths::ConvertRelativePathToFull(ReplayPath);
}
#endif

#endif // USE_SENTRY_NATIVE && !SENTRY_WINGDK
