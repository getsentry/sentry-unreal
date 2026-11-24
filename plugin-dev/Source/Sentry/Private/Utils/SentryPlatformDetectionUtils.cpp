// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "SentryDefines.h"

#include "Misc/FileHelper.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <winternl.h>
#endif

bool FSentryPlatformDetectionUtils::IsRunningUnderWine()
{
#if PLATFORM_WINDOWS
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll != nullptr && GetProcAddress(hNtDll, "wine_get_version") != nullptr)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Detected Wine/Proton compatibility environment"));
		return true;
	}
#endif

	return false;
}

bool FSentryPlatformDetectionUtils::IsSteamDeck()
{
#if PLATFORM_LINUX
	FString Manufacturer;
	FFileHelper::LoadFileToString(Manufacturer, TEXT("/sys/class/dmi/id/sys_vendor"),
		FFileHelper::EHashOptions::None, EFileRead::FILEREAD_Silent);
	Manufacturer.TrimStartAndEndInline();
	if (!Manufacturer.Equals(TEXT("Valve"), ESearchCase::CaseSensitive))
	{
		return false;
	}

	FString Family;
	FFileHelper::LoadFileToString(Family, TEXT("/sys/class/dmi/id/product_family"),
		FFileHelper::EHashOptions::None, EFileRead::FILEREAD_Silent);
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
