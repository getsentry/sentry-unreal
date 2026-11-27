// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IHttpRequest;

enum class ESentryCrashReporterStatus : uint8
{
	Missing = 0,
	Downloading,
	Configured
};

class FSentryCrashReporterDownloader
{
public:
	FSentryCrashReporterDownloader();
	~FSentryCrashReporterDownloader();

	void Download(const TFunction<void(bool)>& OnCompleted);

	ESentryCrashReporterStatus GetStatus() const;

private:
	struct FPlatform
	{
		const TCHAR* Name;
		const TCHAR* RuntimeId;
	};

	bool IsInstalled() const;

	FString GetProperty(const FString& Key) const;

	static FString GetThirdPartyPath(const FString& Platform);
	static FString GetBinariesPath(const FString& Platform);

	bool OverwriteFile(const TArray<uint8>& Data, const FString& FilePath) const;

	static const FPlatform Platforms[];

	const static FString CrashReporterExecName;

	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> DownloadRequests;
};
