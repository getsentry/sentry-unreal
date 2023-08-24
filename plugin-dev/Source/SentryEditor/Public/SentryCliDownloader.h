// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class IHttpRequest;

enum class ESentryCliStatus : uint8
{
	Missing = 0,
	Downloading,
	Configured
};

class FSentryCliDownloader
{
public:
	void Download(const TFunction<void(bool)>& OnCompleted);

	ESentryCliStatus GetStatus();

private:
	FString GetSentryCliPath() const;
	FString GetSentryCliVersion() const;

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> SentryCliDownloadRequest;

	const static FString SentryCliExecName;
};
