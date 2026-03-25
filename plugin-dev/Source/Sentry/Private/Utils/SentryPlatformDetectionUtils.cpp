// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "SentryDefines.h"

#include "Misc/FileHelper.h"

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

bool FSentryPlatformDetectionUtils::IsSteamDeck()
{
#if PLATFORM_LINUX
	FString Manufacturer;
	FFileHelper::LoadFileToString(Manufacturer, TEXT("/sys/class/dmi/id/sys_vendor"));
	Manufacturer.TrimStartAndEndInline();
	if (!Manufacturer.Equals(TEXT("Valve"), ESearchCase::CaseSensitive))
	{
		return false;
	}

	FString Family;
	FFileHelper::LoadFileToString(Family, TEXT("/sys/class/dmi/id/product_family"));
	Family.TrimStartAndEndInline();
	if (Family.Equals(TEXT("Aerith"), ESearchCase::CaseSensitive) ||
		Family.Equals(TEXT("Sephiroth"), ESearchCase::CaseSensitive))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Steam Deck hardware (family: %s)"), *Family);
		return true;
	}
#elif PLATFORM_WINDOWS
	FString Manufacturer;
	const TCHAR* BiosKey = TEXT("Hardware\\Description\\System\\BIOS");
	if (FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, BiosKey, TEXT("SystemManufacturer"), Manufacturer) &&
		Manufacturer.Equals(TEXT("Valve"), ESearchCase::CaseSensitive))
	{
		FString Family;
		if (FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, BiosKey, TEXT("SystemFamily"), Family) &&
			(Family.Equals(TEXT("Aerith"), ESearchCase::CaseSensitive) ||
				Family.Equals(TEXT("Sephiroth"), ESearchCase::CaseSensitive)))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Detected Steam Deck hardware via registry (family: %s)"), *Family);
			return true;
		}
	}
#endif
	return false;
}

bool FSentryPlatformDetectionUtils::IsSteamOS()
{
	TMap<FString, FString> OsRelease = ParseOsRelease();
	const FString* Name = OsRelease.Find(TEXT("NAME"));
	if (Name && Name->Equals(TEXT("SteamOS"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected SteamOS via os-release"));
		return true;
	}
	return false;
}

bool FSentryPlatformDetectionUtils::IsBazzite()
{
	TMap<FString, FString> OsRelease = ParseOsRelease();
	const FString* Name = OsRelease.Find(TEXT("NAME"));
	if (Name && Name->Equals(TEXT("Bazzite"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Bazzite via os-release"));
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

TMap<FString, FString> FSentryPlatformDetectionUtils::ParseOsRelease()
{
	TMap<FString, FString> Fields;

	// Determine the os-release file path.
	// Prefer /run/host/etc/os-release (host OS when in Steam Runtime container),
	// fall back to /etc/os-release.
	FString Contents;
	bool bLoaded = false;

#if PLATFORM_WINDOWS
	// Under Wine/Proton, Z:\ maps to the Linux root filesystem
	bLoaded = FFileHelper::LoadFileToString(Contents, TEXT("Z:/run/host/etc/os-release"));
	if (!bLoaded)
	{
		bLoaded = FFileHelper::LoadFileToString(Contents, TEXT("Z:/etc/os-release"));
	}
#elif PLATFORM_LINUX
	bLoaded = FFileHelper::LoadFileToString(Contents, TEXT("/run/host/etc/os-release"));
	if (!bLoaded)
	{
		bLoaded = FFileHelper::LoadFileToString(Contents, TEXT("/etc/os-release"));
	}
#endif

	if (!bLoaded)
	{
		return Fields;
	}

	// Parse KEY=VALUE or KEY="VALUE" lines
	TArray<FString> Lines;
	Contents.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		FString Trimmed = Line.TrimStartAndEnd();
		if (Trimmed.IsEmpty() || Trimmed.StartsWith(TEXT("#")))
		{
			continue;
		}

		int32 EqPos;
		if (!Trimmed.FindChar(TEXT('='), EqPos))
		{
			continue;
		}

		FString Key = Trimmed.Left(EqPos);
		FString Value = Trimmed.Mid(EqPos + 1);

		// Strip enclosing quotes
		if (Value.Len() >= 2 &&
			((Value.StartsWith(TEXT("\"")) && Value.EndsWith(TEXT("\""))) ||
				(Value.StartsWith(TEXT("'")) && Value.EndsWith(TEXT("'")))))
		{
			Value = Value.Mid(1, Value.Len() - 2);
		}

		Fields.Add(Key, Value);
	}

	return Fields;
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
