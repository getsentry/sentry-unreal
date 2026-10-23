// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlatformDetectionUtils.h"

#include "SentryDefines.h"

#include "Misc/FileHelper.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <winternl.h>
#elif PLATFORM_LINUX
#include <sys/utsname.h>
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

TMap<FString, FSentryVariant> FSentryPlatformDetectionUtils::MakeOsContext(const FString& Name, bool bPreferDistroVersion)
{
	const TMap<FString, FString> OsRelease = ParseOsRelease();
	const FString DistroName = OsRelease.FindRef(TEXT("NAME"));
	const FString DistroVersion = OsRelease.FindRef(TEXT("VERSION_ID"));
	const FString DistroBuild = OsRelease.FindRef(TEXT("BUILD_ID"));
	const FString DistroPrettyName = OsRelease.FindRef(TEXT("PRETTY_NAME"));
	const FString KernelVersion = GetKernelVersion();

	TMap<FString, FSentryVariant> OSContext;
	OSContext.Add(TEXT("type"), TEXT("os"));
	OSContext.Add(TEXT("name"), Name);

	FString Version = bPreferDistroVersion ? DistroVersion : KernelVersion;
	if (Version.IsEmpty())
	{
		Version = bPreferDistroVersion ? KernelVersion : DistroVersion;
	}
	if (!Version.IsEmpty())
	{
		OSContext.Add(TEXT("version"), Version);
	}
	if (!DistroBuild.IsEmpty())
	{
		OSContext.Add(TEXT("build"), DistroBuild);
	}
	if (!KernelVersion.IsEmpty())
	{
		OSContext.Add(TEXT("kernel_version"), KernelVersion);
	}

	if (!DistroName.IsEmpty() && !DistroName.Equals(Name, ESearchCase::IgnoreCase))
	{
		OSContext.Add(TEXT("distribution_name"), DistroName);
		if (!DistroVersion.IsEmpty())
		{
			OSContext.Add(TEXT("distribution_version"), DistroVersion);
		}
		if (!DistroPrettyName.IsEmpty())
		{
			OSContext.Add(TEXT("distribution_pretty_name"), DistroPrettyName);
		}
	}

	return OSContext;
}

FString FSentryPlatformDetectionUtils::GetKernelVersion()
{
#if PLATFORM_WINDOWS
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll == nullptr)
	{
		return FString();
	}

	// wine_get_host_version is exported by Wine's ntdll and reports the host's uname data
	typedef void(CDECL * wine_get_host_version_t)(const char** sysname, const char** release);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4191) // unsafe conversion from FARPROC
#endif
	wine_get_host_version_t wine_get_host_version =
		reinterpret_cast<wine_get_host_version_t>(GetProcAddress(hNtDll, "wine_get_host_version"));
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	if (wine_get_host_version != nullptr)
	{
		const char* SysName = nullptr;
		const char* Release = nullptr;
		wine_get_host_version(&SysName, &Release);
		if (Release != nullptr)
		{
			return FString(UTF8_TO_TCHAR(Release));
		}
	}
#elif PLATFORM_LINUX
	struct utsname UtsName;
	if (uname(&UtsName) == 0)
	{
		return FString(UTF8_TO_TCHAR(UtsName.release));
	}
#endif

	return FString();
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
