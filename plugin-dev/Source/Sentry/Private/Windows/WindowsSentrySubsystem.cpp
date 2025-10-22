// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "WindowsSentrySubsystem.h"

#if USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Windows/Infrastructure/WindowsSentryConverters.h"
#include "Windows/WindowsPlatformStackWalk.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include <winternl.h>
#include "Windows/HideWindowsPlatformTypes.h"

bool FWindowsSentrySubsystem::IsRunningUnderWineOrProton() const
{
	// Check for Wine DLL
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll == nullptr)
	{
		return false;
	}

	// wine_get_version is exported by Wine's ntdll
	typedef const char* (CDECL *wine_get_version_t)(void);
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
	event = FMicrosoftSentrySubsystem::OnCrash(uctx, event, closure);

	if (bIsWineOrProton)
	{
		// Add Wine/Proton context tags
		sentry_value_t contexts = sentry_value_get_by_key(event, "contexts");
		if (sentry_value_is_null(contexts))
		{
			contexts = sentry_value_new_object();
			sentry_value_set_by_key(event, "contexts", contexts);
		}

		sentry_value_t runtime_context = sentry_value_new_object();
		sentry_value_set_by_key(runtime_context, "name", sentry_value_new_string("wine"));
		sentry_value_set_by_key(runtime_context, "proton", sentry_value_new_bool(true));
		sentry_value_set_by_key(contexts, "runtime", runtime_context);

		// Fix OS name to SteamOS for Proton
		sentry_value_t os_context = sentry_value_get_by_key(contexts, "os");
		if (!sentry_value_is_null(os_context))
		{
			// Check if STEAM_COMPAT_DATA_PATH is set (indicates Proton)
			const TCHAR* SteamCompatPath = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_DATA_PATH"));
			if (SteamCompatPath != nullptr && FCString::Strlen(SteamCompatPath) > 0)
			{
				sentry_value_set_by_key(os_context, "name", sentry_value_new_string("SteamOS"));
				UE_LOG(LogSentrySdk, Log, TEXT("Set OS name to SteamOS (detected Proton via STEAM_COMPAT_DATA_PATH)"));
			}
		}

		// Add tags
		sentry_value_t tags = sentry_value_get_by_key(event, "tags");
		if (sentry_value_is_null(tags))
		{
			tags = sentry_value_new_object();
			sentry_value_set_by_key(event, "tags", tags);
		}
		sentry_value_set_by_key(tags, "wine_proton", sentry_value_new_string("true"));
	}

	return event;
}

#endif // USE_SENTRY_NATIVE
