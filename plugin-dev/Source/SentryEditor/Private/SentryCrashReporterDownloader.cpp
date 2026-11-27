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

const FSentryCrashReporterDownloader::FPlatform FSentryCrashReporterDownloader::Platforms[] = {
	{ TEXT("Win64"), TEXT("win-x64") },
	{ TEXT("WinArm64"), TEXT("win-arm64") }
};

const FString FSentryCrashReporterDownloader::CrashReporterExecName = TEXT("Sentry.CrashReporter.exe");

static constexpr float CrashReporterDownloadTimeoutSec = 600.0f;

FSentryCrashReporterDownloader::FSentryCrashReporterDownloader()
	: bIsInstalled(false)
{
	RefreshInstalledState();
}

FSentryCrashReporterDownloader::~FSentryCrashReporterDownloader()
{
	// The HTTP module keeps in-flight requests alive, so their completion callbacks must not outlive this object
	for (const TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& Request : DownloadRequests)
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->CancelRequest();
	}
}

void FSentryCrashReporterDownloader::Download(const TFunction<void(bool)>& OnCompleted)
{
	if (GetStatus() == ESentryCrashReporterStatus::Downloading)
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

	// Shared by all platform requests so that the completion callback fires once after the last one finishes
	struct FDownloadState
	{
		int32 PendingCount;
		bool bSucceeded;
	};

	const TSharedRef<FDownloadState> State = MakeShared<FDownloadState>(FDownloadState{ UE_ARRAY_COUNT(Platforms), true });

	DownloadRequests.Reset();

	for (const FPlatform& Platform : Platforms)
	{
		const FString Url = FString::Printf(TEXT("%s/releases/download/%s/sentry-desktop-crash-reporter-%s-%s.exe"), *Repo, *Version, *Version, Platform.RuntimeId);

		const FString ThirdPartyPath = GetThirdPartyPath(Platform.Name);
		const FString BinariesPath = GetBinariesPath(Platform.Name);

		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

		Request->OnProcessRequestComplete().BindLambda([this, State, ThirdPartyPath, BinariesPath, OnCompleted](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bSuccess)
		{
			const bool bDownloaded = bSuccess && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());

			State->bSucceeded &= bDownloaded && OverwriteFile(Response->GetContent(), ThirdPartyPath) && OverwriteFile(Response->GetContent(), BinariesPath);

			if (--State->PendingCount > 0)
			{
				return;
			}

			RefreshInstalledState();

			OnCompleted(State->bSucceeded && bIsInstalled);
		});

		Request->SetURL(Url);
		Request->SetVerb(TEXT("GET"));
		Request->SetTimeout(CrashReporterDownloadTimeoutSec);

		Request->ProcessRequest();

		DownloadRequests.Add(Request);
	}
}

ESentryCrashReporterStatus FSentryCrashReporterDownloader::GetStatus() const
{
	for (const TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& Request : DownloadRequests)
	{
		if (Request.IsValid() && Request->GetStatus() == EHttpRequestStatus::Processing)
		{
			return ESentryCrashReporterStatus::Downloading;
		}
	}

	return bIsInstalled ? ESentryCrashReporterStatus::Configured : ESentryCrashReporterStatus::Missing;
}

void FSentryCrashReporterDownloader::RefreshInstalledState()
{
	const FString Version = GetProperty(TEXT("version"));

	bIsInstalled = true;

	for (const FPlatform& Platform : Platforms)
	{
		// ThirdParty copy is staged into packaged builds while Binaries copy is used by the editor at runtime
		for (const FString& ExecPath : { GetThirdPartyPath(Platform.Name), GetBinariesPath(Platform.Name) })
		{
			if (!FPaths::FileExists(ExecPath) || !IsPinnedVersion(ExecPath, Version))
			{
				bIsInstalled = false;
				return;
			}
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

bool FSentryCrashReporterDownloader::OverwriteFile(const TArray<uint8>& Data, const FString& FilePath) const
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
uint64 FSentryCrashReporterDownloader::PackVersion(const FString& Version) const
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

bool FSentryCrashReporterDownloader::IsPinnedVersion(const FString& ExecPath, const FString& Version) const
{
#if PLATFORM_WINDOWS
	const uint64 FileVersion = FPlatformMisc::GetFileVersion(ExecPath);
	const uint64 PinnedVersion = PackVersion(Version);
	return FileVersion == 0 || PinnedVersion == 0 || FileVersion == PinnedVersion;
#else
	return true;
#endif
}
