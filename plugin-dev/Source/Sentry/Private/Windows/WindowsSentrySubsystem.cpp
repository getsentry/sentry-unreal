// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Utils/SentryPlatformDetectionUtils.h"

#include "Misc/Paths.h"

void FWindowsSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryBeforeLogHandler* BeforeLogHandler, USentryTraceSampler* TraceSampler)
{
	// Initialize crash logger if enabled
	if (Settings->EnableOnCrashLogging)
	{
		CrashLogger = MakeUnique<FWindowsCrashLogger>();
		if (CrashLogger->IsThreadRunning())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Crash logging enabled - stack traces will be written to game log during crashes"));
		}
		else
		{
			UE_LOG(LogSentrySdk, Warning, TEXT("Crash logging requested but thread failed to initialize"));
			CrashLogger.Reset();
		}
	}

	// Detect Wine/Proton before initializing
	WineProtonInfo = FSentryPlatformDetectionUtils::DetectWineProton();

	// Call parent implementation
	FMicrosoftSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, BeforeLogHandler, TraceSampler);

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
	// If crash logging is enabled, log the crash callstack to game log
	if (CrashLogger)
	{
		// Get a pseudo-handle to the current thread (the crashed thread)
		// We need a real handle for cross-thread stack walking
		HANDLE CurrentThreadPseudoHandle = GetCurrentThread();
		HANDLE CrashedThreadHandle = nullptr;

		// Duplicate the pseudo-handle to get a real handle
		if (DuplicateHandle(
				GetCurrentProcess(),	   // Source process
				CurrentThreadPseudoHandle, // Source handle (pseudo)
				GetCurrentProcess(),	   // Target process
				&CrashedThreadHandle,	   // Target handle (real)
				0,						   // Desired access (ignored when using DUPLICATE_SAME_ACCESS)
				Windows::FALSE,			   // Inherit handle
				DUPLICATE_SAME_ACCESS	   // Options
				))
		{
			// Log the crash (with timeout to prevent hanging)
			// This waits for the logging thread to complete stack walking, fill GErrorHist and dump callstack to logs
			CrashLogger->LogCrash(uctx, CrashedThreadHandle, 5000);

			// Close the duplicated handle
			CloseHandle(CrashedThreadHandle);
		}
	}

	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

void FWindowsSentrySubsystem::Close()
{
	if (CrashLogger)
	{
		CrashLogger.Reset();
	}

	FMicrosoftSentrySubsystem::Close();
}

#endif // USE_SENTRY_NATIVE
