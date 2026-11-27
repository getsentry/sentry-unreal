// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryCrashReporterDownloader.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if UE_VERSION_OLDER_THAN(5, 0, 0)
#include "HAL/PlatformFilemanager.h"
#else
#include "HAL/PlatformFileManager.h"
#endif

namespace
{
struct FSentryCrashReporterPlatform
{
	const TCHAR* Name;
	const TCHAR* RuntimeId;
};

const FSentryCrashReporterPlatform CrashReporterPlatforms[] = {
	{ TEXT("Win64"), TEXT("win-x64") },
	{ TEXT("WinArm64"), TEXT("win-arm64") }
};

const TCHAR* CrashReporterExecName = TEXT("Sentry.CrashReporter.exe");

constexpr float CrashReporterDownloadTimeoutSec = 600.0f;

bool OverwriteFile(const TArray<uint8>& Data, const FString& FilePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const FString TempPath = FilePath + TEXT(".download");

	if (!FFileHelper::SaveArrayToFile(Data, *TempPath))
	{
		PlatformFile.DeleteFile(*TempPath);
		return false;
	}

	if ((PlatformFile.FileExists(*FilePath) && !PlatformFile.DeleteFile(*FilePath)) || !PlatformFile.MoveFile(*FilePath, *TempPath))
	{
		PlatformFile.DeleteFile(*TempPath);
		return false;
	}

	return true;
}

#if PLATFORM_WINDOWS
// Packs the numeric part of a version string the same way as the executable's file version (16 bits per component)
uint64 PackVersion(const FString& Version)
{
	uint64 Result = 0;
	int32 Components = 0;
	uint64 Component = 0;

	for (const TCHAR Char : Version)
	{
		if (FChar::IsDigit(Char))
		{
			Component = Component * 10 + (Char - TEXT('0'));
		}
		else if (Char == TEXT('.') && Components < 3)
		{
			Result = (Result << 16) | (Component & 0xFFFF);
			Component = 0;
			++Components;
		}
		else
		{
			break;
		}
	}

	Result = (Result << 16) | (Component & 0xFFFF);
	return Result << (16 * (3 - Components));
}
#endif

// An executable left over from another plugin version doesn't match the pinned one
bool IsPinnedVersion(const FString& ExecPath, const FString& Version)
{
#if PLATFORM_WINDOWS
	const uint64 FileVersion = FPlatformMisc::GetFileVersion(ExecPath);
	const uint64 PinnedVersion = PackVersion(Version);
	return FileVersion == 0 || PinnedVersion == 0 || FileVersion == PinnedVersion;
#else
	return true;
#endif
}
} // namespace

FSentryCrashReporterDownloader::FSentryCrashReporterDownloader()
	: PendingDownloads(0)
	, bDownloadFailed(false)
	, bIsInstalled(false)
{
	RefreshInstalledState();
}

void FSentryCrashReporterDownloader::Download(const TFunction<void(bool)>& OnCompleted)
{
	if (PendingDownloads > 0)
	{
		return;
	}

	const FString Repo = GetProperty(TEXT("repo"));
	const FString Version = GetProperty(TEXT("version"));

	if (Repo.IsEmpty() || Version.IsEmpty())
	{
		OnCompleted(false);
		return;
	}

	OnDownloadCompleted = OnCompleted;
	PendingDownloads = UE_ARRAY_COUNT(CrashReporterPlatforms);
	bDownloadFailed = false;

	for (const FSentryCrashReporterPlatform& Platform : CrashReporterPlatforms)
	{
		const FString Url = FString::Printf(TEXT("%s/releases/download/%s/sentry-desktop-crash-reporter-%s-%s.exe"), *Repo, *Version, *Version, Platform.RuntimeId);

		const FString ThirdPartyPath = GetThirdPartyPath(Platform.Name);
		const FString BinariesPath = GetBinariesPath(Platform.Name);

		const TWeakPtr<FSentryCrashReporterDownloader> WeakSelf = AsShared();

		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

		Request->OnProcessRequestComplete().BindLambda([WeakSelf, ThirdPartyPath, BinariesPath](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bSuccess)
		{
			const bool bDownloaded = bSuccess && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());

			const bool bInstalled = bDownloaded && OverwriteFile(Response->GetContent(), ThirdPartyPath) && OverwriteFile(Response->GetContent(), BinariesPath);

			if (const TSharedPtr<FSentryCrashReporterDownloader> This = WeakSelf.Pin())
			{
				This->OnPlatformCompleted(bInstalled);
			}
		});

		Request->SetURL(Url);
		Request->SetVerb(TEXT("GET"));
		Request->SetTimeout(CrashReporterDownloadTimeoutSec);

		Request->ProcessRequest();
	}
}

ESentryCrashReporterStatus FSentryCrashReporterDownloader::GetStatus() const
{
	if (PendingDownloads > 0)
	{
		return ESentryCrashReporterStatus::Downloading;
	}

	return bIsInstalled ? ESentryCrashReporterStatus::Configured : ESentryCrashReporterStatus::Missing;
}

void FSentryCrashReporterDownloader::OnPlatformCompleted(bool bSuccess)
{
	bDownloadFailed |= !bSuccess;

	if (--PendingDownloads > 0)
	{
		return;
	}

	RefreshInstalledState();

	if (OnDownloadCompleted)
	{
		OnDownloadCompleted(!bDownloadFailed && bIsInstalled);
	}
}

void FSentryCrashReporterDownloader::RefreshInstalledState()
{
	const FString Version = GetProperty(TEXT("version"));

	bIsInstalled = true;

	for (const FSentryCrashReporterPlatform& Platform : CrashReporterPlatforms)
	{
		const FString ExecPath = GetThirdPartyPath(Platform.Name);

		if (!FPaths::FileExists(ExecPath) || !IsPinnedVersion(ExecPath, Version))
		{
			bIsInstalled = false;
			return;
		}
	}
}

FString FSentryCrashReporterDownloader::GetProperty(const FString& Key) const
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	const FString PropertiesPath = FPaths::Combine(PluginDir, TEXT("sentry-crash-reporter.properties"));

	TArray<FString> PropertiesContents;
	FFileHelper::LoadFileToStringArray(PropertiesContents, *PropertiesPath);

	const FString Prefix = Key + TEXT("=");
	for (const FString& Line : PropertiesContents)
	{
		const FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.StartsWith(Prefix))
		{
			return TrimmedLine.RightChop(Prefix.Len()).TrimStartAndEnd();
		}
	}

	return FString();
}

FString FSentryCrashReporterDownloader::GetThirdPartyPath(const FString& Platform)
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), Platform, CrashReporterExecName));
}

FString FSentryCrashReporterDownloader::GetBinariesPath(const FString& Platform)
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, TEXT("Binaries"), Platform, CrashReporterExecName));
}
