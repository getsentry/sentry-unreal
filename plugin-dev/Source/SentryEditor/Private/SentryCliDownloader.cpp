// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryCliDownloader.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"

#if PLATFORM_WINDOWS
const FString FSentryCliDownloader::SentryCliExecName = TEXT("sentry-cli-Windows-x86_64.exe");
#elif PLATFORM_MAC
const FString FSentryCliDownloader::SentryCliExecName = TEXT("sentry-cli-Darwin-universal");
#elif PLATFORM_LINUX
const FString FSentryCliDownloader::SentryCliExecName = TEXT("sentry-cli-Linux-x86_64");
#endif

void FSentryCliDownloader::Download(const TFunction<void(bool)>& OnCompleted)
{
	SentryCliDownloadRequest = FHttpModule::Get().CreateRequest();

	SentryCliDownloadRequest->OnProcessRequestComplete().BindLambda([=](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
	{
		if (!bSuccess || !Response.IsValid())
		{
			OnCompleted(false);
			return;
		}

		FString SentryCliDirPath, SentryCliFilename, SentryCliExtension;
		FPaths::Split(GetSentryCliPath(), SentryCliDirPath, SentryCliFilename, SentryCliExtension);

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!PlatformFile.DirectoryExists(*SentryCliDirPath))
		{
			if (!PlatformFile.CreateDirectoryTree(*SentryCliDirPath))
			{
				OnCompleted(false);
				return;
			}
		}

		FFileHelper::SaveArrayToFile(Response->GetContent(), *GetSentryCliPath());

		OnCompleted(true);
	});

	SentryCliDownloadRequest->SetURL(FString::Printf(TEXT("https://github.com/getsentry/sentry-cli/releases/download/%s/%s"), TEXT("2.20.5"), *SentryCliExecName));
	SentryCliDownloadRequest->SetVerb(TEXT("GET"));

	SentryCliDownloadRequest->ProcessRequest();
}

ESentryCliStatus FSentryCliDownloader::GetStatus()
{
	if(SentryCliDownloadRequest.IsValid() && SentryCliDownloadRequest->GetStatus() == EHttpRequestStatus::Processing)
	{
		return ESentryCliStatus::Downloading;
	}

	if(FPaths::FileExists(GetSentryCliPath()))
	{
		return ESentryCliStatus::Configured;
	}

	return ESentryCliStatus::Missing;
}

FString FSentryCliDownloader::GetSentryCliPath() const
{
	const FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Sentry"))->GetBaseDir();

	return FPaths::Combine(PluginDir, TEXT("Source"), TEXT("ThirdParty"), TEXT("CLI"), SentryCliExecName);
}
