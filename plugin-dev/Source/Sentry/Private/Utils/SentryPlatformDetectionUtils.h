// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentryPlatformInfo.h"

/**
 * Utility class for detecting platform-specific runtime environments
 * including Wine/Proton, Linux distributions, and handheld devices
 */
class FSentryPlatformDetectionUtils
{
public:
	/**
	 * Detects if running under Wine or Proton (Windows binaries on Linux)
	 * Works by checking for Wine's ntdll.dll exports and environment variables
	 *
	 * @return Wine/Proton detection information
	 */
	static FWineProtonInfo DetectWineProton();

	/**
	 * Detects if running on Steam Deck hardware using SMBIOS/DMI data.
	 * Checks manufacturer "Valve" with product family "Aerith" (LCD) or "Sephiroth" (OLED).
	 * Works on native Linux (via /sys/class/dmi/id/) and on Windows under Wine/Proton (via registry).
	 *
	 * @return true if Steam Deck hardware is detected
	 */
	static bool IsSteamDeck();

	/**
	 * Detects if running on SteamOS by parsing /etc/os-release
	 *
	 * @return true if SteamOS is detected
	 */
	static bool IsSteamOS();

	/**
	 * Detects if running on Bazzite by parsing /etc/os-release
	 *
	 * @return true if Bazzite is detected
	 */
	static bool IsBazzite();

	/**
	 * Checks if running under Steam (any platform)
	 *
	 * @return true if Steam environment is detected
	 */
	static bool IsRunningSteam();

	/**
	 * Gets the runtime name for Sentry context based on Wine/Proton info
	 * Returns "Proton" or "Wine"
	 */
	static FString GetRuntimeName(const FWineProtonInfo& WineProtonInfo);

	/**
	 * Gets the runtime version for Sentry context based on Wine/Proton info
	 * Returns Proton build name or Wine version
	 */
	static FString GetRuntimeVersion(const FWineProtonInfo& WineProtonInfo);

private:
	/** Parses Wine version string to extract version and Proton information */
	static void ParseWineVersion(const FString& VersionString, FWineProtonInfo& OutInfo);

	/**
	 * Parses /etc/os-release (or /run/host/etc/os-release) and returns a map of key-value pairs.
	 * On Windows under Wine/Proton, reads from the Z: drive mapping.
	 */
	static TMap<FString, FString> ParseOsRelease();
};
