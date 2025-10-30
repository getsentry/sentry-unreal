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
