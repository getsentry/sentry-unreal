// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "SentryDefines.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
		PRAGMA_DISABLE_UNSAFE_TYPECAST_WARNINGS
		wine_get_version_t wine_get_version =
			reinterpret_cast<wine_get_version_t>(GetProcAddress(hNtDll, "wine_get_version"));
		PRAGMA_RESTORE_UNSAFE_TYPECAST_WARNINGS

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

void FSentryPlatformDetectionUtils::SetSentryOSContext(const FLinuxDistroInfo& DistroInfo)
{
#if USE_SENTRY_NATIVE
	if (DistroInfo.ID.IsEmpty())
	{
		return;
	}

	sentry_value_t os_context = sentry_value_new_object();

	// OS name should be high-level: "SteamOS", "Bazzite", or "Linux"
	FString OSName = TEXT("Linux");
	if (DistroInfo.bIsSteamOS)
	{
		OSName = TEXT("SteamOS");
	}
	else if (DistroInfo.bIsBazzite)
	{
		OSName = TEXT("Bazzite");
	}

	sentry_value_set_by_key(os_context, "name",
		sentry_value_new_string(TCHAR_TO_UTF8(*OSName)));

	if (!DistroInfo.Version.IsEmpty())
	{
		sentry_value_set_by_key(os_context, "version",
			sentry_value_new_string(TCHAR_TO_UTF8(*DistroInfo.Version)));
	}

	// Use distribution object for detailed Linux distro info (standard Sentry field)
	sentry_value_t distribution = sentry_value_new_object();
	sentry_value_set_by_key(distribution, "name",
		sentry_value_new_string(TCHAR_TO_UTF8(*DistroInfo.ID)));

	if (!DistroInfo.PrettyName.IsEmpty())
	{
		sentry_value_set_by_key(distribution, "pretty_name",
			sentry_value_new_string(TCHAR_TO_UTF8(*DistroInfo.PrettyName)));
	}

	if (!DistroInfo.Version.IsEmpty())
	{
		sentry_value_set_by_key(distribution, "version",
			sentry_value_new_string(TCHAR_TO_UTF8(*DistroInfo.Version)));
	}

	sentry_value_set_by_key(os_context, "distribution", distribution);

	// Add custom fields for gaming distro detection
	if (DistroInfo.bIsGamingDistro)
	{
		sentry_value_set_by_key(os_context, "gaming_distro", sentry_value_new_bool(true));
	}

	sentry_set_context("os", os_context);
	UE_LOG(LogSentrySdk, Log, TEXT("Set OS context to %s (%s)"), *OSName, *DistroInfo.PrettyName);
#endif
}

void FSentryPlatformDetectionUtils::SetSentryDeviceContext(const FHandheldDeviceInfo& HandheldInfo)
{
#if USE_SENTRY_NATIVE
	if (!HandheldInfo.bIsHandheld)
	{
		return;
	}

	sentry_value_t device_context = sentry_value_new_object();

	// Use standard Sentry device fields
	if (!HandheldInfo.Manufacturer.IsEmpty())
	{
		sentry_value_set_by_key(device_context, "manufacturer",
			sentry_value_new_string(TCHAR_TO_UTF8(*HandheldInfo.Manufacturer)));
	}

	if (!HandheldInfo.Model.IsEmpty())
	{
		sentry_value_set_by_key(device_context, "model",
			sentry_value_new_string(TCHAR_TO_UTF8(*HandheldInfo.Model)));
	}

	// Set device_type to indicate it's a handheld
	sentry_value_set_by_key(device_context, "device_type", sentry_value_new_string("handheld"));

	// Add custom fields for specific handheld types
	if (HandheldInfo.bIsSteamDeck)
	{
		sentry_value_set_by_key(device_context, "steam_deck", sentry_value_new_bool(true));

		if (HandheldInfo.bIsSteamDeckOLED)
		{
			sentry_value_set_by_key(device_context, "steam_deck_oled", sentry_value_new_bool(true));
		}
	}

	if (!HandheldInfo.Codename.IsEmpty())
	{
		sentry_value_set_by_key(device_context, "model_id",
			sentry_value_new_string(TCHAR_TO_UTF8(*HandheldInfo.Codename)));
	}

	sentry_set_context("device", device_context);
	UE_LOG(LogSentrySdk, Log, TEXT("Set device context: %s %s"),
		*HandheldInfo.Manufacturer, *HandheldInfo.Model);
#endif
}

void FSentryPlatformDetectionUtils::SetSentryRuntimeContext(const FWineProtonInfo& WineProtonInfo)
{
#if USE_SENTRY_NATIVE
	if (!WineProtonInfo.bIsRunningUnderWine)
	{
		return;
	}

	// Add runtime context with detailed information
	sentry_value_t runtime_context = sentry_value_new_object();

	// Set name and version based on whether it's Proton or Wine
	if (WineProtonInfo.bIsProton)
	{
		sentry_value_set_by_key(runtime_context, "name", sentry_value_new_string("Proton"));

		// Use Proton build name if available, otherwise fall back to Wine version
		if (!WineProtonInfo.ProtonBuildName.IsEmpty())
		{
			sentry_value_set_by_key(runtime_context, "version",
				sentry_value_new_string(TCHAR_TO_UTF8(*WineProtonInfo.ProtonBuildName)));
		}
		else if (!WineProtonInfo.Version.IsEmpty())
		{
			sentry_value_set_by_key(runtime_context, "version",
				sentry_value_new_string(TCHAR_TO_UTF8(*WineProtonInfo.Version)));
		}

		if (WineProtonInfo.bIsExperimental)
		{
			sentry_value_set_by_key(runtime_context, "raw_description", sentry_value_new_string("Experimental Proton Build"));
		}
	}
	else
	{
		sentry_value_set_by_key(runtime_context, "name", sentry_value_new_string("Wine"));

		if (!WineProtonInfo.Version.IsEmpty())
		{
			sentry_value_set_by_key(runtime_context, "version",
				sentry_value_new_string(TCHAR_TO_UTF8(*WineProtonInfo.Version)));
		}
	}

	sentry_set_context("runtime", runtime_context);
	UE_LOG(LogSentrySdk, Log, TEXT("Set %s runtime context"),
		WineProtonInfo.bIsProton ? TEXT("Proton") : TEXT("Wine"));
#endif
}

void FSentryPlatformDetectionUtils::SetSentryPlatformTags(const FWineProtonInfo* WineProtonInfo,
	const FLinuxDistroInfo* DistroInfo, const FHandheldDeviceInfo* HandheldInfo)
{
#if USE_SENTRY_NATIVE
	// Add Wine/Proton tags
	if (WineProtonInfo && WineProtonInfo->bIsRunningUnderWine)
	{
		if (WineProtonInfo->bIsProton)
		{
			sentry_set_tag("proton", "true");
		} else {
			sentry_set_tag("wine", "true");
		}
	}

	// Add distribution tags
	if (DistroInfo)
	{
		if (DistroInfo->bIsSteamOS)
		{
			sentry_set_tag("steamos", "true");
		}

		if (DistroInfo->bIsBazzite)
		{
			sentry_set_tag("bazzite", "true");
		}

		if (DistroInfo->bIsGamingDistro)
		{
			sentry_set_tag("gaming_distro", "true");
		}
	}

	// Add handheld device tags
	if (HandheldInfo && HandheldInfo->bIsHandheld)
	{
		sentry_set_tag("handheld", "true");

		if (HandheldInfo->bIsSteamDeck)
		{
			sentry_set_tag("steam_deck", "true");
		}
	}

	UE_LOG(LogSentrySdk, Log, TEXT("Set platform detection tags"));
#endif
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
