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
