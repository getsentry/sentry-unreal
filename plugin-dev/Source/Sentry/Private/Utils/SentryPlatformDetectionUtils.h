// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentryVariant.h"

/**
 * Utility class for detecting platform-specific runtime environments
 * including Wine/Proton, Linux distributions, and handheld devices
 */
class FSentryPlatformDetectionUtils
{
public:
	/**
	 * Detects if running under Wine or Proton (Windows binaries on Linux)
	 * Works by checking for Wine's ntdll.dll exports
	 *
	 * @return true if Wine or Proton is detected
	 */
	static bool IsRunningUnderWine();

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
	 * Builds an OS context for the Linux host using os-release data and the kernel version.
	 * Works on native Linux and on Windows under Wine/Proton.
	 *
	 * @param Name OS name to report (e.g. "SteamOS", "Bazzite", "Linux")
	 * @param bPreferDistroVersion Whether to report the distro version instead of the kernel version
	 */
	static TMap<FString, FSentryVariant> MakeOsContext(const FString& Name, bool bPreferDistroVersion);

private:
	/** Gets the Linux kernel release of the host */
	static FString GetKernelVersion();

	/**
	 * Parses /etc/os-release (or /run/host/etc/os-release) and returns a map of key-value pairs.
	 * On Windows under Wine/Proton, reads from the Z: drive mapping.
	 */
	static TMap<FString, FString> ParseOsRelease();
};
