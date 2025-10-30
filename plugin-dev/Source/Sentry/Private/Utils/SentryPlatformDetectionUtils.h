// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Platform detection information for Wine/Proton environments
 */
struct FWineProtonInfo
{
	/** Whether Wine or Proton is detected */
	bool bIsRunningUnderWine = false;

	/** Whether running specifically under Proton (Steam's Wine fork) */
	bool bIsProton = false;

	/** Wine/Proton version string (e.g., "8.0-3" for Proton, "9.0" for Wine) */
	FString Version;

	/** Proton build name (e.g., "Proton 8.0", "Proton Experimental") */
	FString ProtonBuildName;

	/** Whether this is an experimental Proton build */
	bool bIsExperimental = false;
};

/**
 * Linux distribution information
 */
struct FLinuxDistroInfo
{
	/** Distribution name (e.g., "SteamOS", "Arch Linux", "Ubuntu", "Bazzite") */
	FString Name;

	/** Distribution ID (e.g., "steamos", "arch", "ubuntu", "bazzite") */
	FString ID;

	/** Distribution version (e.g., "3.5.7", "22.04") */
	FString Version;

	/** Distribution pretty name (e.g., "SteamOS Holo") */
	FString PrettyName;

	/** Variant information (e.g., "holo" for SteamOS) */
	FString Variant;

	/** Whether this is a gaming-focused distribution */
	bool bIsGamingDistro = false;

	/** Whether this is SteamOS specifically */
	bool bIsSteamOS = false;

	/** Whether this is Bazzite specifically */
	bool bIsBazzite = false;
};

/**
 * Handheld device information
 */
struct FHandheldDeviceInfo
{
	/** Whether running on a handheld device */
	bool bIsHandheld = false;

	/** Device manufacturer (e.g., "Valve", "ASUS", "Lenovo") */
	FString Manufacturer;

	/** Device model (e.g., "Steam Deck", "ROG Ally", "Legion Go") */
	FString Model;

	/** Device codename (e.g., "Jupiter", "Galileo") */
	FString Codename;

	/** Whether this is a Steam Deck */
	bool bIsSteamDeck = false;

	/** Whether this is a Steam Deck OLED */
	bool bIsSteamDeckOLED = false;
};

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
	 * Detects the Linux distribution by reading /etc/os-release
	 * Only applicable on Linux platforms
	 *
	 * @return Linux distribution information
	 */
	static FLinuxDistroInfo DetectLinuxDistro();

	/**
	 * Detects if running on a handheld gaming device
	 * Checks DMI information, device tree, and Steam-specific environment variables
	 *
	 * @return Handheld device information
	 */
	static FHandheldDeviceInfo DetectHandheldDevice();

	/**
	 * Checks if running under Steam (any platform)
	 *
	 * @return true if Steam environment is detected
	 */
	static bool IsRunningSteam();

	/**
	 * Sets Sentry OS context based on detected Linux distribution
	 * This uses the standard Sentry OS context schema
	 *
	 * @param DistroInfo The detected Linux distribution information
	 */
	static void SetSentryOSContext(const FLinuxDistroInfo& DistroInfo);

	/**
	 * Sets Sentry device context based on detected handheld device
	 * This uses the standard Sentry device context schema
	 *
	 * @param HandheldInfo The detected handheld device information
	 */
	static void SetSentryDeviceContext(const FHandheldDeviceInfo& HandheldInfo);

	/**
	 * Sets Sentry runtime context based on detected Wine/Proton
	 * This uses the standard Sentry runtime context schema
	 *
	 * @param WineProtonInfo The detected Wine/Proton information
	 */
	static void SetSentryRuntimeContext(const FWineProtonInfo& WineProtonInfo);

	/**
	 * Sets Sentry tags based on platform detection
	 *
	 * @param WineProtonInfo Wine/Proton detection info (optional)
	 * @param DistroInfo Linux distribution info (optional)
	 * @param HandheldInfo Handheld device info (optional)
	 */
	static void SetSentryPlatformTags(const FWineProtonInfo* WineProtonInfo,
		const FLinuxDistroInfo* DistroInfo, const FHandheldDeviceInfo* HandheldInfo);

private:
	/** Reads and parses /etc/os-release file */
	static TMap<FString, FString> ParseOsReleaseFile();

	/** Reads a file and returns its contents as a string */
	static FString ReadFileToString(const FString& FilePath);

	/** Parses Wine version string to extract version and Proton information */
	static void ParseWineVersion(const FString& VersionString, FWineProtonInfo& OutInfo);

	/** Reads DMI information from sysfs */
	static FString ReadDMIInfo(const FString& DMIField);

	/** Detects Steam Deck specifically */
	static void DetectSteamDeck(FHandheldDeviceInfo& OutInfo);

	/** Detects other handheld devices (ROG Ally, Legion Go, etc.) */
	static void DetectOtherHandhelds(FHandheldDeviceInfo& OutInfo);
};
