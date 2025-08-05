// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "Mac/MacSentrySubsystem.h"

#include "AppleSentryId.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void FMacSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler)
{
	FAppleSentrySubsystem::InitWithSettings(settings, beforeSendHandler, beforeBreadcrumbHandler, traceSampler);

	if (IsEnabled() && isScreenshotAttachmentEnabled)
	{
		FCoreDelegates::OnHandleSystemError.AddLambda([this]()
		{
			TryCaptureScreenshot();
		});
	}
}

TSharedPtr<ISentryId> FMacSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	TSharedPtr<ISentryId> id = FAppleSentrySubsystem::CaptureEnsure(type, message);

	if (isScreenshotAttachmentEnabled)
	{
		const FString& screenshotPath = TryCaptureScreenshot();
		UploadScreenshotForEvent(id, screenshotPath);
	}

	return id;
}

FString FMacSentrySubsystem::TryCaptureScreenshot() const
{
	const FString ScreenshotPath = GetScreenshotPath();
	SentryScreenshotUtils::CaptureScreenshot(ScreenshotPath);
	return ScreenshotPath;
}

FString FMacSentrySubsystem::GetGameLogPath() const
{
	return SentryFileUtils::GetGameLogPath();
}

FString FMacSentrySubsystem::GetLatestGameLog() const
{
	return SentryFileUtils::GetGameLogBackupPath();
}
