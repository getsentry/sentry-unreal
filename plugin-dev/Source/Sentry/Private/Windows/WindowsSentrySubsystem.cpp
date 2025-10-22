// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <winternl.h>

bool FWindowsSentrySubsystem::IsRunningUnderWineOrProton() const
{
	// Check for Wine DLL
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll == nullptr)
	{
		return false;
	}

	// wine_get_version is exported by Wine's ntdll
	typedef const char*(CDECL * wine_get_version_t)(void);
	wine_get_version_t wine_get_version =
		reinterpret_cast<wine_get_version_t>(GetProcAddress(hNtDll, "wine_get_version"));

	if (wine_get_version != nullptr)
	{
		const char* version = wine_get_version();
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine version: %hs"), version);
		return true;
	}

	// Check environment variable (common in Proton)
	const TCHAR* WineVersion = FPlatformMisc::GetEnvironmentVariable(TEXT("WINE_VERSION"));
	if (WineVersion != nullptr && FCString::Strlen(WineVersion) > 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine/Proton via WINE_VERSION environment variable: %s"), WineVersion);
		return true;
	}

	return false;
}

void FWindowsSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler, USentryBeforeLogHandler* BeforeLogHandler, USentryTraceSampler* TraceSampler)
{
	// Detect Wine/Proton before initializing
	bIsWineOrProton = IsRunningUnderWineOrProton();

	// Call parent implementation
	FMicrosoftSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, BeforeLogHandler, TraceSampler);

	// Add Wine/Proton context for all events if detected
	if (bIsWineOrProton && IsEnabled())
	{
		// Add runtime context
		sentry_value_t runtime_context = sentry_value_new_object();
		sentry_value_set_by_key(runtime_context, "name", sentry_value_new_string("wine"));
		sentry_value_set_by_key(runtime_context, "proton", sentry_value_new_bool(true));
		sentry_set_context("runtime", runtime_context);

		// Check if running under Proton and set OS context
		const TCHAR* SteamCompatPath = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_DATA_PATH"));
		if (SteamCompatPath != nullptr && FCString::Strlen(SteamCompatPath) > 0)
		{
			// Running under Proton - set OS to SteamOS
			sentry_value_t os_context = sentry_value_new_object();
			sentry_value_set_by_key(os_context, "name", sentry_value_new_string("SteamOS"));
			sentry_set_context("os", os_context);
			UE_LOG(LogSentrySdk, Log, TEXT("Set OS context to SteamOS (detected Proton via STEAM_COMPAT_DATA_PATH)"));
		}

		// Add tag for filtering
		sentry_set_tag("wine_proton", "true");
		UE_LOG(LogSentrySdk, Log, TEXT("Added Wine/Proton context and tags for all events"));
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
	if (bIsWineOrProton)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Enabling Crashpad stack capture adjustment for Wine/Proton compatibility"));
		sentry_options_set_crashpad_adjust_stack_capture(Options, 1);
	}
}

sentry_value_t FWindowsSentrySubsystem::OnCrash(const sentry_ucontext_t* uctx, sentry_value_t event, void* closure)
{
	// Context and tags are already set globally during InitWithSettings
	// Just call parent implementation
	return FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);
}

#endif // USE_SENTRY_NATIVE
