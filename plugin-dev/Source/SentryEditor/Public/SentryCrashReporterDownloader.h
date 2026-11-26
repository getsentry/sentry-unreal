// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#if PLATFORM_WINDOWS && !UE_VERSION_OLDER_THAN(5, 2, 0)
#define SENTRY_CRASH_REPORTER_DOWNLOAD_SUPPORTED 1
#else
#define SENTRY_CRASH_REPORTER_DOWNLOAD_SUPPORTED 0
#endif

#if SENTRY_CRASH_REPORTER_DOWNLOAD_SUPPORTED

enum class ESentryCrashReporterStatus : uint8
{
	Missing = 0,
	Downloading,
	Configured
};

/** Downloads Sentry Crash Reporter executables for Windows (x64 and ARM64) at the version pinned by the plugin. */
class FSentryCrashReporterDownloader : public TSharedFromThis<FSentryCrashReporterDownloader>
{
public:
	FSentryCrashReporterDownloader();

	void Download(const TFunction<void(bool)>& OnCompleted);

	ESentryCrashReporterStatus GetStatus() const;

private:
	void OnPlatformCompleted(bool bSuccess);

	void RefreshInstalledState();

	FString GetProperty(const FString& Key) const;

	static FString GetThirdPartyPath(const FString& Platform);
	static FString GetBinariesPath(const FString& Platform);

	TFunction<void(bool)> OnDownloadCompleted;

	int32 PendingDownloads;
	bool bDownloadFailed;
	bool bIsInstalled;
};

#endif
