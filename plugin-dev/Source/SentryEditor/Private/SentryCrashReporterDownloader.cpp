// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryCrashReporterDownloader.h"

#if SENTRY_CRASH_REPORTER_DOWNLOAD_SUPPORTED

#include "Async/Async.h"
#include "FileUtilities/ZipArchiveReader.h"
#include "HAL/PlatformFileManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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

bool InstallExecutable(const TArray<uint8>& Archive, const FString& ArchivePath, const FString& ThirdPartyPath, const FString& BinariesPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!FFileHelper::SaveArrayToFile(Archive, *ArchivePath))
	{
		return false;
	}

	TArray<uint8> Executable;
	bool bExtracted = false;
	{
		const FZipArchiveReader ArchiveReader(PlatformFile.OpenRead(*ArchivePath));
		bExtracted = ArchiveReader.IsValid() && ArchiveReader.TryReadFile(CrashReporterExecName, Executable);
	}

	PlatformFile.DeleteFile(*ArchivePath);

	return bExtracted && OverwriteFile(Executable, ThirdPartyPath) && OverwriteFile(Executable, BinariesPath);
}

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

	const FString ArchiveDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("Sentry")));

	for (const FSentryCrashReporterPlatform& Platform : CrashReporterPlatforms)
	{
		const FString ArchiveName = FString::Printf(TEXT("sentry-desktop-crash-reporter-%s-%s.zip"), *Version, Platform.RuntimeId);
		const FString Url = FString::Printf(TEXT("%s/releases/download/%s/%s"), *Repo, *Version, *ArchiveName);

		const FString ArchivePath = FPaths::Combine(ArchiveDir, ArchiveName);
		const FString ThirdPartyPath = GetThirdPartyPath(Platform.Name);
		const FString BinariesPath = GetBinariesPath(Platform.Name);

		const TWeakPtr<FSentryCrashReporterDownloader> WeakSelf = AsShared();

		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

		Request->OnProcessRequestComplete().BindLambda([WeakSelf, ArchivePath, ThirdPartyPath, BinariesPath](FHttpRequestPtr HttpRequest, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
			{
				if (const TSharedPtr<FSentryCrashReporterDownloader> This = WeakSelf.Pin())
				{
					This->OnPlatformCompleted(false);
				}
				return;
			}

			Async(EAsyncExecution::ThreadPool, [WeakSelf, Response, ArchivePath, ThirdPartyPath, BinariesPath]()
			{
				const bool bInstalled = InstallExecutable(Response->GetContent(), ArchivePath, ThirdPartyPath, BinariesPath);

				AsyncTask(ENamedThreads::GameThread, [WeakSelf, bInstalled]()
				{
					if (const TSharedPtr<FSentryCrashReporterDownloader> This = WeakSelf.Pin())
					{
						This->OnPlatformCompleted(bInstalled);
					}
				});
			});
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
	const uint64 ExpectedVersion = PackVersion(GetProperty(TEXT("version")));

	bIsInstalled = true;

	for (const FSentryCrashReporterPlatform& Platform : CrashReporterPlatforms)
	{
		const FString ExecPath = GetThirdPartyPath(Platform.Name);

		if (!FPaths::FileExists(ExecPath))
		{
			bIsInstalled = false;
			return;
		}

		// An executable left over from another plugin version doesn't match the pinned one
		const uint64 FileVersion = FPlatformMisc::GetFileVersion(ExecPath);
		if (FileVersion != 0 && ExpectedVersion != 0 && FileVersion != ExpectedVersion)
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

#endif
