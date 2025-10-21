// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <windows.h>
#endif

/**
 * Utility class for detecting and handling Wine/Proton/SteamOS environments.
 *
 * When games built for Windows run on Linux through Proton (Valve's Wine-based compatibility layer),
 * certain features like Crashpad crash handlers may not work correctly due to incomplete Windows API
 * emulation. This utility helps detect such environments and provides workarounds.
 */
class FSentryProtonUtils
{
public:
	/**
	 * Information about the Wine/Proton environment
	 */
	struct FProtonInfo
	{
		/** True if running under Wine/Proton */
		bool bIsRunningUnderWine = false;

		/** Wine version string (e.g., "8.0") */
		FString WineVersion;

		/** Host operating system name (e.g., "Linux") */
		FString HostSystem;

		/** Host OS release version */
		FString HostRelease;

		/** True if Steam Proton environment variables are detected */
		bool bIsSteamProton = false;

		/** Proton version if running under Steam Proton */
		FString ProtonVersion;

		/** Steam compatibility data path */
		FString SteamCompatDataPath;
	};

	/**
	 * Detects if the application is running under Wine/Proton.
	 *
	 * This uses the wine_get_version() function exported by Wine's ntdll.dll,
	 * which is the standard method for Wine detection. Also checks for Steam Proton
	 * specific environment variables.
	 *
	 * @return Information about the Wine/Proton environment
	 */
	static FProtonInfo DetectProtonEnvironment();

	/**
	 * Converts a Windows path to be Wine/Proton compatible.
	 *
	 * In Wine/Proton:
	 * - The Linux root filesystem (/) is mapped to Z:\
	 * - Windows drive letters still work for Wine prefix drives
	 *
	 * @param WindowsPath The Windows-style path to convert
	 * @return Converted path suitable for Wine/Proton, or original path if not under Wine
	 */
	static FString ConvertPathForWine(const FString& WindowsPath);

	/**
	 * Checks if running under Steam Proton specifically (as opposed to regular Wine).
	 *
	 * @return True if Steam Proton environment variables are present
	 */
	static bool IsRunningSteamProton();

private:
	/** Cached detection result to avoid repeated checks */
	static FProtonInfo CachedProtonInfo;

	/** Whether detection has been performed */
	static bool bHasDetected;

	/** Critical section for thread-safe detection */
	static FCriticalSection DetectionLock;
};
