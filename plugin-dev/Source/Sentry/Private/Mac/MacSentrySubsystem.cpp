// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "Mac/MacSentrySubsystem.h"

#include "AppleSentryId.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "Utils/SentryFileUtils.h"

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
	NSWindow* MainWindow = [NSApp mainWindow];
	if (!MainWindow)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("No main window found!"));
		return FString("");
	}

	NSRect WindowRect = [MainWindow frame];
	CGWindowID WindowID = (CGWindowID)[MainWindow windowNumber];
	CGImageRef ScreenshotRef = CGWindowListCreateImage(WindowRect, kCGWindowListOptionIncludingWindow, WindowID, kCGWindowImageDefault);

	if (!ScreenshotRef)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to capture screenshot - invalid ScreenshotRef."));
		return FString("");
	}

	NSBitmapImageRep* BitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:ScreenshotRef];
	NSData* ImageData = [BitmapRep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];

	TArray<uint8> ImageBytes;
	uint32 SavedSize = (uint32)[ImageData length];
	ImageBytes.AddUninitialized(SavedSize);
	FPlatformMemory::Memcpy(ImageBytes.GetData(), [ImageData bytes], SavedSize);

	CGImageRelease(ScreenshotRef);

	FString ScreenshotPath = GetScreenshotPath();

	if (!FFileHelper::SaveArrayToFile(ImageBytes, *ScreenshotPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot to: %s"), *ScreenshotPath);
		return FString("");
	}

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
