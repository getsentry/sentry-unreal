// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SentryDefines.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <winternl.h>
#include "Windows/HideWindowsPlatformTypes.h"
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

FLinuxDistroInfo FSentryPlatformDetectionUtils::DetectLinuxDistro()
{
	FLinuxDistroInfo Info;

	// Note: When running under Wine/Proton, this will access the actual Linux host's /etc/os-release
	// Wine maps the Unix filesystem, so this works for both native Linux and Wine/Proton scenarios
	TMap<FString, FString> OsRelease = ParseOsReleaseFile();

	if (OsRelease.Num() > 0)
	{
		// Extract standard fields
		Info.Name = OsRelease.FindRef(TEXT("NAME")).TrimQuotes();
		Info.ID = OsRelease.FindRef(TEXT("ID")).TrimQuotes();
		Info.Version = OsRelease.FindRef(TEXT("VERSION_ID")).TrimQuotes();
		Info.PrettyName = OsRelease.FindRef(TEXT("PRETTY_NAME")).TrimQuotes();
		Info.Variant = OsRelease.FindRef(TEXT("VARIANT_ID")).TrimQuotes();

		// Detect specific distributions
		FString IDLower = Info.ID.ToLower();
		Info.bIsSteamOS = IDLower.Contains(TEXT("steamos"));
		Info.bIsBazzite = IDLower.Contains(TEXT("bazzite"));

		// Check for gaming distros
		TArray<FString> GamingDistros = {
			TEXT("steamos"),
			TEXT("bazzite"),
			TEXT("chimeraos"),
			TEXT("nobara"),
			TEXT("garuda"),
			TEXT("drauger")
		};

		for (const FString& GamingDistro : GamingDistros)
		{
			if (IDLower.Contains(GamingDistro))
			{
				Info.bIsGamingDistro = true;
				break;
			}
		}

		UE_LOG(LogSentrySdk, Log, TEXT("Detected Linux distribution: %s (ID: %s, Version: %s)"),
			*Info.PrettyName, *Info.ID, *Info.Version);

		if (Info.bIsGamingDistro)
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Gaming-focused distribution detected"));
		}
	}

	return Info;
}

FHandheldDeviceInfo FSentryPlatformDetectionUtils::DetectHandheldDevice()
{
	FHandheldDeviceInfo Info;

	// First, try to detect Steam Deck
	DetectSteamDeck(Info);

	// If not a Steam Deck, try detecting other handhelds
	if (!Info.bIsHandheld)
	{
		DetectOtherHandhelds(Info);
	}

	if (Info.bIsHandheld)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected handheld device: %s %s"), *Info.Manufacturer, *Info.Model);
		if (!Info.Codename.IsEmpty())
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Device codename: %s"), *Info.Codename);
		}
	}

	return Info;
}

bool FSentryPlatformDetectionUtils::IsRunningSteam()
{
	// Check for Steam-specific environment variables
	FString SteamAppId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamAppId"));
	FString SteamGameId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamGameId"));
	FString SteamOverlayGameId = FPlatformMisc::GetEnvironmentVariable(TEXT("SteamOverlayGameId"));

	return !SteamAppId.IsEmpty() || !SteamGameId.IsEmpty() || !SteamOverlayGameId.IsEmpty();
}

FString FSentryPlatformDetectionUtils::GetOSNameForContext(const FLinuxDistroInfo& DistroInfo)
{
	// OS name should be high-level: "SteamOS", "Bazzite", or "Linux"
	if (DistroInfo.bIsSteamOS)
	{
		return TEXT("SteamOS");
	}
	else if (DistroInfo.bIsBazzite)
	{
		return TEXT("Bazzite");
	}
	return TEXT("Linux");
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

TMap<FString, FString> FSentryPlatformDetectionUtils::ParseOsReleaseFile()
{
	TMap<FString, FString> Result;

	// Try to read /etc/os-release
	// On native Linux, this reads the local file
	// On Wine/Proton, Wine maps Unix paths so this reads the host Linux's file
	FString OsReleaseContent = ReadFileToString(TEXT("/etc/os-release"));

	if (OsReleaseContent.IsEmpty())
	{
		// Try alternative location
		OsReleaseContent = ReadFileToString(TEXT("/usr/lib/os-release"));
	}

	if (!OsReleaseContent.IsEmpty())
	{
		TArray<FString> Lines;
		OsReleaseContent.ParseIntoArrayLines(Lines);

		for (const FString& Line : Lines)
		{
			// Skip comments and empty lines
			FString TrimmedLine = Line.TrimStartAndEnd();
			if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT("#")))
			{
				continue;
			}

			// Parse KEY=VALUE format
			int32 EqualsIndex;
			if (TrimmedLine.FindChar('=', EqualsIndex))
			{
				FString Key = TrimmedLine.Left(EqualsIndex).TrimStartAndEnd();
				FString Value = TrimmedLine.Mid(EqualsIndex + 1).TrimStartAndEnd();
				Result.Add(Key, Value);
			}
		}
	}

	return Result;
}

FString FSentryPlatformDetectionUtils::ReadFileToString(const FString& FilePath)
{
	FString Content;

	if (FPaths::FileExists(FilePath))
	{
		if (!FFileHelper::LoadFileToString(Content, *FilePath))
		{
			Content.Empty();
		}
	}

	return Content;
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

FString FSentryPlatformDetectionUtils::ReadDMIInfo(const FString& DMIField)
{
	FString Result;

	// DMI information is available through /sys on Linux
	// Wine/Proton maps this correctly from the host system
	FString DMIPath = FString::Printf(TEXT("/sys/class/dmi/id/%s"), *DMIField);
	Result = ReadFileToString(DMIPath).TrimStartAndEnd();

	return Result;
}

void FSentryPlatformDetectionUtils::DetectSteamDeck(FHandheldDeviceInfo& OutInfo)
{
	// Check DMI information for Steam Deck
	FString ProductName = ReadDMIInfo(TEXT("product_name"));
	FString SystemVendor = ReadDMIInfo(TEXT("sys_vendor"));

	if (ProductName.Contains(TEXT("Jupiter"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.bIsSteamDeck = true;
		OutInfo.Manufacturer = TEXT("Valve");
		OutInfo.Model = TEXT("Steam Deck");
		OutInfo.Codename = ProductName;

		// Galileo is the OLED model
		if (ProductName.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
		{
			OutInfo.bIsSteamDeckOLED = true;
			OutInfo.Model = TEXT("Steam Deck OLED");
		}
		return;
	}

	// Check for Steam Deck via device tree (alternative method)
	FString DeviceTreeModel = ReadFileToString(TEXT("/sys/firmware/devicetree/base/model"));
	if (DeviceTreeModel.Contains(TEXT("Jupiter"), ESearchCase::IgnoreCase) ||
		DeviceTreeModel.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.bIsSteamDeck = true;
		OutInfo.Manufacturer = TEXT("Valve");
		OutInfo.Model = TEXT("Steam Deck");
		OutInfo.Codename = DeviceTreeModel.TrimStartAndEnd();

		if (DeviceTreeModel.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
		{
			OutInfo.bIsSteamDeckOLED = true;
			OutInfo.Model = TEXT("Steam Deck OLED");
		}
		return;
	}

	// Check for SteamOS-specific chassis type and board name
	FString ChassisType = ReadDMIInfo(TEXT("chassis_type"));
	if (ChassisType.Equals(TEXT("3"))) // Type 3 = Desktop, but Steam Deck reports this
	{
		// Additional check: Steam Deck will have specific board name
		FString BoardName = ReadDMIInfo(TEXT("board_name"));
		if (BoardName.Contains(TEXT("Jupiter"), ESearchCase::IgnoreCase) ||
			BoardName.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
		{
			OutInfo.bIsHandheld = true;
			OutInfo.bIsSteamDeck = true;
			OutInfo.Manufacturer = TEXT("Valve");
			OutInfo.Model = TEXT("Steam Deck");
			OutInfo.Codename = BoardName;

			if (BoardName.Contains(TEXT("Galileo"), ESearchCase::IgnoreCase))
			{
				OutInfo.bIsSteamDeckOLED = true;
				OutInfo.Model = TEXT("Steam Deck OLED");
			}
		}
	}
}

void FSentryPlatformDetectionUtils::DetectOtherHandhelds(FHandheldDeviceInfo& OutInfo)
{
	FString ProductName = ReadDMIInfo(TEXT("product_name"));
	FString SystemVendor = ReadDMIInfo(TEXT("sys_vendor"));
	FString BoardName = ReadDMIInfo(TEXT("board_name"));

	// ASUS ROG Ally
	if (ProductName.Contains(TEXT("ROG Ally"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("RC71L"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = TEXT("ASUS");
		OutInfo.Model = TEXT("ROG Ally");
		return;
	}

	// Lenovo Legion Go
	if (ProductName.Contains(TEXT("Legion Go"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("83E1"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = TEXT("Lenovo");
		OutInfo.Model = TEXT("Legion Go");
		return;
	}

	// AYANEO devices
	if (SystemVendor.Contains(TEXT("AYANEO"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("AYANEO"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = TEXT("AYANEO");
		OutInfo.Model = ProductName.TrimStartAndEnd();
		return;
	}

	// GPD devices (GPD Win, GPD Win Max, etc.)
	if (SystemVendor.Contains(TEXT("GPD"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("GPD"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = TEXT("GPD");
		OutInfo.Model = ProductName.TrimStartAndEnd();
		return;
	}

	// OneXPlayer devices
	if (SystemVendor.Contains(TEXT("ONE-NETBOOK"), ESearchCase::IgnoreCase) ||
		ProductName.Contains(TEXT("ONEXPLAYER"), ESearchCase::IgnoreCase))
	{
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = TEXT("OneXPlayer");
		OutInfo.Model = ProductName.TrimStartAndEnd();
		return;
	}

	// Check chassis type as fallback (type 30 = Tablet, type 31 = Convertible)
	FString ChassisType = ReadDMIInfo(TEXT("chassis_type"));
	if (ChassisType.Equals(TEXT("30")) || ChassisType.Equals(TEXT("31")))
	{
		// This might be a handheld, but we're not sure which one
		OutInfo.bIsHandheld = true;
		OutInfo.Manufacturer = SystemVendor.TrimStartAndEnd();
		OutInfo.Model = ProductName.TrimStartAndEnd();
	}
}
