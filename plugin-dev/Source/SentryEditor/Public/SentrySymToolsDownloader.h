// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IHttpRequest;

enum class ESentrySymToolsStatus : uint8
{
	Missing = 0,
	Downloading,
	Configured
};

class FSentrySymToolsDownloader
{
public:
	void Download(const TFunction<void(bool)>& OnCompleted);

	ESentrySymToolsStatus GetStatus();

private:
	void Download(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>& Request, const FString& Url, const FString& SavePath, const TFunction<void(bool)>& OnCompleted);

	FString GetSentryCliPath() const;
	FString GetSentryCliVersion() const;
	FString GetSymUploadScriptPath() const;

	bool HasExecutePermission(const FString& FilePath) const;
	bool SetExecutePermission(const FString& FilePath) const;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> SentryCliDownloadRequest;
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> SentryScriptDownloadRequest;

	const static FString SentryCliExecName;
	const static FString SentrySymUploadScriptName;
};
