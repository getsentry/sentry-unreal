// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "SentryDefines.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <winternl.h>
#endif

FWineProtonInfo FSentryPlatformDetectionUtils::DetectWineProton()
{
	FWineProtonInfo Info;

#if PLATFORM_WINDOWS
	// Check for Wine DLL on Windows builds running under Wine/Proton
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll != nullptr)
	{
		// wine_get_version is exported by Wine's ntdll
		typedef const char*(CDECL * wine_get_version_t)(void);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191) // unsafe conversion from FARPROC
#endif
		wine_get_version_t wine_get_version =
			reinterpret_cast<wine_get_version_t>(GetProcAddress(hNtDll, "wine_get_version"));
#ifdef _MSC_VER
#pragma warning(pop)
#endif

		if (wine_get_version != nullptr)
		{
			const char* version = wine_get_version();
			Info.bIsRunningUnderWine = true;
			Info.Version = FString(version);
			ParseWineVersion(Info.Version, Info);

			UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine version: %s"), *Info.Version);
		}
	}

	// Check environment variables (common in Proton)
	if (!Info.bIsRunningUnderWine)
	{
		FString WineVersion = FPlatformMisc::GetEnvironmentVariable(TEXT("WINE_VERSION"));
		if (!WineVersion.IsEmpty())
		{
			Info.bIsRunningUnderWine = true;
			Info.Version = WineVersion;
			ParseWineVersion(Info.Version, Info);

			UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine/Proton via WINE_VERSION environment variable: %s"), *Info.Version);
		}
	}

	// Check for Proton-specific environment variables
	if (Info.bIsRunningUnderWine)
	{
		FString SteamCompatPath = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_DATA_PATH"));
		FString SteamCompatTool = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_COMPAT_CLIENT_INSTALL_PATH"));

		if (!SteamCompatPath.IsEmpty() || !SteamCompatTool.IsEmpty())
		{
			Info.bIsProton = true;
			UE_LOG(LogSentrySdk, Log, TEXT("Detected Proton environment"));
		}

		// Try to get Proton build name from PROTON_VERSION environment variable
		FString ProtonVersion = FPlatformMisc::GetEnvironmentVariable(TEXT("PROTON_VERSION"));
		if (!ProtonVersion.IsEmpty())
		{
			Info.ProtonBuildName = ProtonVersion;
			Info.bIsExperimental = ProtonVersion.Contains(TEXT("Experimental"), ESearchCase::IgnoreCase);
		}
	}
#endif

	return Info;
}

bool FSentryPlatformDetectionUtils::IsSteamOS()
{
	// Check for SteamOS-specific environment variables
	// SteamOS sets these environment variables in its gaming mode
	FString SteamOSVar = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOS"));
	FString SteamGameMode = FPlatformMisc::GetEnvironmentVariable(TEXT("STEAM_RUNTIME"));

	// Check if SteamOS variable is explicitly set
	if (!SteamOSVar.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected SteamOS via SteamOS environment variable"));
		return true;
	}

	// Check for Steam Runtime which is commonly set on SteamOS
	if (SteamGameMode.Contains(TEXT("steamrt")) || SteamGameMode.Contains(TEXT("steam-runtime")))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected SteamOS via STEAM_RUNTIME environment variable"));
		return true;
	}

	return false;
}

bool FSentryPlatformDetectionUtils::IsBazzite()
{
	// Bazzite sets specific environment variables
	FString ImageName = FPlatformMisc::GetEnvironmentVariable(TEXT("IMAGE_NAME"));
	FString ImageVendor = FPlatformMisc::GetEnvironmentVariable(TEXT("IMAGE_VENDOR"));
	FString ImageFlavor = FPlatformMisc::GetEnvironmentVariable(TEXT("IMAGE_FLAVOR"));

	// Check for Bazzite-specific image name
	if (ImageName.Contains(TEXT("bazzite"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Bazzite via IMAGE_NAME environment variable"));
		return true;
	}

	// Check for Bazzite vendor
	if (ImageVendor.Contains(TEXT("bazzite"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Bazzite via IMAGE_VENDOR environment variable"));
		return true;
	}

	return false;
}

bool FSentryPlatformDetectionUtils::IsRunningSteam()
{
	// Check for Steam-specific environment variables
	FString SteamAppId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamAppId"));
	FString SteamGameId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamGameId"));
	FString SteamOverlayGameId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOverlayGameId"));

	return !SteamAppId.IsEmpty() || !SteamGameId.IsEmpty() || !SteamOverlayGameId.IsEmpty();
}

FString FSentryPlatformDetectionUtils::GetRuntimeName(const FWineProtonInfo& WineProtonInfo)
{
	return WineProtonInfo.bIsProton ? TEXT("Proton") : TEXT("Wine");
}

FString FSentryPlatformDetectionUtils::GetRuntimeVersion(const FWineProtonInfo& WineProtonInfo)
{
	// For Proton, use build name if available, otherwise fall back to Wine version
	if (WineProtonInfo.bIsProton && !WineProtonInfo.ProtonBuildName.IsEmpty())
	{
		return WineProtonInfo.ProtonBuildName;
	}
	return WineProtonInfo.Version;
}

void FSentryPlatformDetectionUtils::ParseWineVersion(const FString& VersionString, FWineProtonInfo& OutInfo)
{
	// Wine versions typically look like:
	// - "9.0" (standard Wine)
	// - "8.0-3" (Proton)
	// - "7.0-rc1" (Wine release candidate)

	// Check if this looks like a Proton version (contains dash with number after it)
	if (VersionString.Contains(TEXT("-")))
	{
		TArray<FString> Parts;
		VersionString.ParseIntoArray(Parts, TEXT("-"));

		if (Parts.Num() >= 2)
		{
			// Check if the part after dash is numeric (Proton style) or GE build
			if (Parts[1].IsNumeric() || Parts[1].Equals(TEXT("GE"), ESearchCase::IgnoreCase))
			{
				OutInfo.bIsProton = true;

				// Construct Proton build name if not already set
				if (OutInfo.ProtonBuildName.IsEmpty())
				{
					// Generate a default Proton name
					if (Parts[1].Equals(TEXT("GE"), ESearchCase::IgnoreCase))
					{
						OutInfo.ProtonBuildName = FString::Printf(TEXT("Proton-GE %s"), *VersionString);
					}
					else
					{
						OutInfo.ProtonBuildName = FString::Printf(TEXT("Proton %s"), *Parts[0]);
					}
				}
			}
		}
	}
}
