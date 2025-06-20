// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySymToolsDownloader.h"
#include "SentryModule.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Launch/Resources/Version.h"

#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if UE_VERSION_OLDER_THAN(5, 0, 0)
#include "HAL/PlatformFilemanager.h"
#else
#include "HAL/PlatformFileManager.h"
#endif

#if PLATFORM_WINDOWS
const FString FSentrySymToolsDownloader::SentryCliExecName = TEXT("sentry-cli-Windows-x86_64.exe");
const FString FSentrySymToolsDownloader::SentrySymUploadScriptName = TEXT("upload-debug-symbols-win.bat");
#elif PLATFORM_MAC
const FString FSentrySymToolsDownloader::SentryCliExecName = TEXT("sentry-cli-Darwin-universal");
const FString FSentrySymToolsDownloader::SentrySymUploadScriptName = TEXT("upload-debug-symbols.sh");
#elif PLATFORM_LINUX
const FString FSentrySymToolsDownloader::SentryCliExecName = TEXT("sentry-cli-Linux-x86_64");
const FString FSentrySymToolsDownloader::SentrySymUploadScriptName = TEXT("upload-debug-symbols.sh");
#endif

void FSentrySymToolsDownloader::Download(const TFunction<void(bool)>& OnCompleted)
{
	SentryCliDownloadRequest = FHttpModule::Get().CreateRequest();
	SentryScriptDownloadRequest = FHttpModule::Get().CreateRequest();

	const FString SentryCliExecPath = GetSentryCliPath();
	const FString CliDownloadUrl = FString::Printf(TEXT("https://github.com/getsentry/sentry-cli/releases/download/%s/%s"), *GetSentryCliVersion(), *SentryCliExecName);

	const FString PluginVersion = FSentryModule::Get().GetPluginVersion();
	const FString SymUploadScriptPath = GetSymUploadScriptPath();
	const FString SymUploadScriptDownloadUrl = FString::Printf(TEXT("https://raw.githubusercontent.com/getsentry/sentry-unreal/refs/tags/%s/plugin-dev/Scripts/%s"), *PluginVersion, *SentrySymUploadScriptName);

	Download(SentryCliDownloadRequest, CliDownloadUrl, SentryCliExecPath, OnCompleted);
	Download(SentryScriptDownloadRequest, SymUploadScriptDownloadUrl, SymUploadScriptPath, OnCompleted);
}

ESentrySymToolsStatus FSentrySymToolsDownloader::GetStatus()
{
	if ((SentryCliDownloadRequest.IsValid() && SentryCliDownloadRequest->GetStatus() == EHttpRequestStatus::Processing) ||
		(SentryScriptDownloadRequest.IsValid() && SentryScriptDownloadRequest->GetStatus() == EHttpRequestStatus::Processing))
	{
		return ESentrySymToolsStatus::Downloading;
	}

	if (FPaths::FileExists(GetSentryCliPath()) && FPaths::FileExists(GetSymUploadScriptPath()) &&
		HasExecutePermission(GetSentryCliPath()) && HasExecutePermission(GetSymUploadScriptPath()))
	{
		return ESentrySymToolsStatus::Configured;
	}

	return ESentrySymToolsStatus::Missing;
}

void FSentrySymToolsDownloader::Download(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& Request, const FString& Url, const FString& SavePath, const TFunction<void(bool)>& OnCompleted)
{
	Request->OnProcessRequestComplete().BindLambda([this, SavePath, OnCompleted](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		if (!bSuccess || !Response.IsValid())
		{
			OnCompleted(false);
			return;
		}

		FString DirPath, Filename, Extension;
		FPaths::Split(SavePath, DirPath, Filename, Extension);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		if (!PlatformFile.DirectoryExists(*DirPath))
		{
			if (!PlatformFile.CreateDirectoryTree(*DirPath))
			{
				OnCompleted(false);
				return;
			}
		}

		if (PlatformFile.FileExists(*SavePath))
		{
			if (!PlatformFile.DeleteFile(*SavePath))
			{
				OnCompleted(false);
				return;
			}
		}

		FFileHelper::SaveArrayToFile(Response->GetContent(), *SavePath);

#if PLATFORM_LINUX || PLATFORM_MAC
		if (!SetExecutePermission(SavePath))
		{
			OnCompleted(false);
			return;
		}
#endif

		if (GetStatus() == ESentrySymToolsStatus::Configured)
		{
			OnCompleted(true);
		}
	});

	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));

	Request->ProcessRequest();
}

FString FSentrySymToolsDownloader::GetSentryCliPath() const
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	return FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("CLI"), SentryCliExecName);
}

FString FSentrySymToolsDownloader::GetSentryCliVersion() const
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	const FString SentryCliPropertiesPath = FPaths::Combine(PluginDir, TEXT("sentry-cli.properties"));

	TArray<FString> CliPropertiesContents;
	FFileHelper::LoadFileToStringArray(CliPropertiesContents, *SentryCliPropertiesPath);

	FString SentryCliVersion;
	FParse::Value(*CliPropertiesContents[0], TEXT("version="), SentryCliVersion);

	return SentryCliVersion;
}

FString FSentrySymToolsDownloader::GetSymUploadScriptPath() const
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();
	return FPaths::Combine(PluginDir, TEXT("Scripts"), SentrySymUploadScriptName);
}

bool FSentrySymToolsDownloader::HasExecutePermission(const FString& FilePath) const
{
#if PLATFORM_LINUX || PLATFORM_MAC
	struct stat FileInfo;
	if (stat(TCHAR_TO_UTF8(*FilePath), &FileInfo) == 0)
	{
		return (FileInfo.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
	}

	return false;
#else
	// No-op on Windows
	return true;
#endif
}

bool FSentrySymToolsDownloader::SetExecutePermission(const FString& FilePath) const
{
#if PLATFORM_LINUX || PLATFORM_MAC
	struct stat FileInfo;
	if (stat(TCHAR_TO_UTF8(*FilePath), &FileInfo) == 0)
	{
		return chmod(TCHAR_TO_UTF8(*FilePath), FileInfo.st_mode | S_IXUSR | S_IXGRP | S_IXOTH) == 0;
	}

	return false;
#else
	// No-op on Windows
	return true;
#endif
}
