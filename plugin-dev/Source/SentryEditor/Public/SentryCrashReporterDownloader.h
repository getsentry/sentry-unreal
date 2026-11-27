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

	void Download(const TFunction<void(bool)>& OnCompleted);

	ESentryCrashReporterStatus GetStatus() const;

private:
	struct FPlatform
	{
		const TCHAR* Name;
		const TCHAR* RuntimeId;
	};

	void RefreshInstalledState();

	FString GetProperty(const FString& Key) const;

	static FString GetThirdPartyPath(const FString& Platform);
	static FString GetBinariesPath(const FString& Platform);

	bool OverwriteFile(const TArray<uint8>& Data, const FString& FilePath) const;

	bool IsPinnedVersion(const FString& ExecPath, const FString& Version) const;

#if PLATFORM_WINDOWS
	// Packs the numeric part of a version string the same way as the executable's file version (16 bits per component)
	uint64 PackVersion(const FString& Version) const;
#endif

	static const FPlatform Platforms[];

	const static FString CrashReporterExecName;

	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> DownloadRequests;

	bool bIsInstalled;
};
