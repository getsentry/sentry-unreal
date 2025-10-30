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
	 * Gets the OS name for Sentry context based on distribution info
	 * Returns "SteamOS", "Bazzite", or "Linux"
	 */
	static FString GetOSNameForContext(const FLinuxDistroInfo& DistroInfo);

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
