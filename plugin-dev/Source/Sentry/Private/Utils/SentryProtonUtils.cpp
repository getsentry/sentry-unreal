// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryProtonUtils.h"

#include "SentryDefines.h"

#include "HAL/PlatformMisc.h"

// Static member initialization
FSentryProtonUtils::FProtonInfo FSentryProtonUtils::CachedProtonInfo;
bool FSentryProtonUtils::bHasDetected = false;
FCriticalSection FSentryProtonUtils::DetectionLock;

FSentryProtonUtils::FProtonInfo FSentryProtonUtils::DetectProtonEnvironment()
{
	FScopeLock Lock(&DetectionLock);

	// Return cached result if already detected
	if (bHasDetected)
	{
		return CachedProtonInfo;
	}

	FProtonInfo Info;

#if PLATFORM_WINDOWS
	// Check for Wine detection via wine_get_version function
	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	if (hNtdll)
	{
		// wine_get_version returns the Wine version string
		typedef const char* (CDECL *wine_get_version_t)(void);
		wine_get_version_t wine_get_version = (wine_get_version_t)GetProcAddress(hNtdll, "wine_get_version");

		if (wine_get_version)
		{
			Info.bIsRunningUnderWine = true;
			const char* version = wine_get_version();
			if (version)
			{
				Info.WineVersion = FString(ANSI_TO_TCHAR(version));
				UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine version: %s"), *Info.WineVersion);
			}

			// wine_get_host_version provides information about the host OS
			typedef void (CDECL *wine_get_host_version_t)(const char** sysname, const char** release);
			wine_get_host_version_t wine_get_host_version = (wine_get_host_version_t)GetProcAddress(hNtdll, "wine_get_host_version");

			if (wine_get_host_version)
			{
				const char* sysname = nullptr;
				const char* release = nullptr;
				wine_get_host_version(&sysname, &release);

				if (sysname)
				{
					Info.HostSystem = FString(ANSI_TO_TCHAR(sysname));
					UE_LOG(LogSentrySdk, Log, TEXT("Detected host system: %s"), *Info.HostSystem);
				}

				if (release)
				{
					Info.HostRelease = FString(ANSI_TO_TCHAR(release));
					UE_LOG(LogSentrySdk, Log, TEXT("Detected host release: %s"), *Info.HostRelease);
				}
			}
		}
	}

	// Check for Steam Proton specific environment variables
	FString ProtonVersion = FPlatformMisc::GetEnvironmentVariable(TEXT("PROTON_VERSION"));
	FString SteamCompatData = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_DATA_PATH"));
	FString SteamAppId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamAppId"));

	if (!ProtonVersion.IsEmpty() || !SteamCompatData.IsEmpty())
	{
		Info.bIsSteamProton = true;
		Info.ProtonVersion = ProtonVersion;
		Info.SteamCompatDataPath = SteamCompatData;

		UE_LOG(LogSentrySdk, Log, TEXT("Detected Steam Proton environment"));
		if (!ProtonVersion.IsEmpty())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("  Proton Version: %s"), *ProtonVersion);
		}
		if (!SteamCompatData.IsEmpty())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("  Compat Data Path: %s"), *SteamCompatData);
		}
		if (!SteamAppId.IsEmpty())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("  Steam App ID: %s"), *SteamAppId);
		}

		// If we detected Steam Proton but not Wine, mark as Wine anyway
		// (Steam Proton always uses Wine)
		if (!Info.bIsRunningUnderWine)
		{
			Info.bIsRunningUnderWine = true;
			Info.WineVersion = ProtonVersion.IsEmpty() ? TEXT("Unknown (Proton)") : ProtonVersion;
			UE_LOG(LogSentrySdk, Log, TEXT("Marking as Wine environment due to Proton detection"));
		}
	}

	// Log final detection result
	if (Info.bIsRunningUnderWine)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Running under Wine/Proton compatibility layer. Some features (like Crashpad) may not work correctly."));
	}
	else
	{
		UE_LOG(LogSentrySdk, Verbose, TEXT("Not running under Wine/Proton - native Windows environment detected."));
	}
#endif

	// Cache the result
	CachedProtonInfo = Info;
	bHasDetected = true;

	return Info;
}

FString FSentryProtonUtils::ConvertPathForWine(const FString& WindowsPath)
{
#if PLATFORM_WINDOWS
	FProtonInfo Info = DetectProtonEnvironment();

	if (Info.bIsRunningUnderWine)
	{
		// Wine handles Windows paths natively in most cases
		// The Z: drive maps to the Linux root filesystem (/)
		// Standard Windows drive letters (C:, D:, etc.) map to the Wine prefix

		// For most cases, Wine's path translation handles this automatically
		// However, we ensure the path is absolute to avoid issues
		FString ConvertedPath = FPaths::ConvertRelativePathToFull(WindowsPath);

		UE_LOG(LogSentrySdk, Verbose, TEXT("Wine path conversion: %s -> %s"), *WindowsPath, *ConvertedPath);

		return ConvertedPath;
	}
#endif

	return WindowsPath;
}

bool FSentryProtonUtils::IsRunningSteamProton()
{
	FProtonInfo Info = DetectProtonEnvironment();
	return Info.bIsSteamProton;
}
