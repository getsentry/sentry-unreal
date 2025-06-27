// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FIOSSentrySubsystem : public FAppleSentrySubsystem
{
public:
	virtual void InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler) override;

	virtual void HandleAssert() override;

	virtual FString TryCaptureScreenshot() const override;

	bool IsScreenshotEnabled() const { return isScreenshotAttachmentEnabled; }

protected:
	virtual FString GetGameLogPath() const override;
	virtual FString GetLatestGameLog() const override;

private:
	FString NormalizeToPublicIOSPath(const FString& logFilePath) const;
};

typedef FIOSSentrySubsystem FPlatformSentrySubsystem;
