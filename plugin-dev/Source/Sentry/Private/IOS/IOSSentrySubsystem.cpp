// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "IOS/IOSSentrySubsystem.h"

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryScreenshotUtils.h"

#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// In UE 5.5 and newer, PLATFORM_VISIONOS may be incorrectly set to true during iOS packaging
// causing build errors due to engine's IOSAppDelegate.h including visionOS-specific headers.
#pragma push_macro("PLATFORM_VISIONOS")

#undef PLATFORM_VISIONOS
#define PLATFORM_VISIONOS 0

#include "IOS/IOSAppDelegate.h"

#pragma pop_macro("PLATFORM_VISIONOS")

static FIOSSentrySubsystem* GIOSSentrySubsystem = nullptr;

struct sigaction DefaultSigIllHandler;
struct sigaction DefaultSigEmtHandler;
struct sigaction DefaultSigFpeHandler;
struct sigaction DefaultSigBusHandler;
struct sigaction DefaultSigSegvHandler;
struct sigaction DefaultSigSysHandler;

void SaveDefaultSignalHandlers()
{
	sigaction(SIGILL, NULL, &DefaultSigIllHandler);
	sigaction(SIGEMT, NULL, &DefaultSigEmtHandler);
	sigaction(SIGFPE, NULL, &DefaultSigFpeHandler);
	sigaction(SIGBUS, NULL, &DefaultSigBusHandler);
	sigaction(SIGSEGV, NULL, &DefaultSigSegvHandler);
	sigaction(SIGSYS, NULL, &DefaultSigSysHandler);
}

void RestoreDefaultSignalHandlers()
{
	sigaction(SIGILL, &DefaultSigIllHandler, NULL);
	sigaction(SIGEMT, &DefaultSigEmtHandler, NULL);
	sigaction(SIGFPE, &DefaultSigFpeHandler, NULL);
	sigaction(SIGBUS, &DefaultSigBusHandler, NULL);
	sigaction(SIGSEGV, &DefaultSigSegvHandler, NULL);
	sigaction(SIGSYS, &DefaultSigSysHandler, NULL);
}

static void IOSSentrySignalHandler(int Signal, siginfo_t* Info, void* Context)
{
	if (GIOSSentrySubsystem && GIOSSentrySubsystem->IsEnabled() && GIOSSentrySubsystem->IsScreenshotEnabled())
	{
		GIOSSentrySubsystem->TryCaptureScreenshot();
	}

	RestoreDefaultSignalHandlers();

	// Re-raise signal to default handler
	raise(Signal);
}

void InstallSentrySignalHandler()
{
	struct sigaction Action;
	memset(&Action, 0, sizeof(Action));
	Action.sa_sigaction = IOSSentrySignalHandler;
	Action.sa_flags = SA_SIGINFO | SA_ONSTACK;

	sigaction(SIGILL, &Action, NULL);
	sigaction(SIGEMT, &Action, NULL);
	sigaction(SIGFPE, &Action, NULL);
	sigaction(SIGBUS, &Action, NULL);
	sigaction(SIGSEGV, &Action, NULL);
	sigaction(SIGSYS, &Action, NULL);
}

void FIOSSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler)
{
	GIOSSentrySubsystem = this;

	SaveDefaultSignalHandlers();
	InstallSentrySignalHandler();

	FAppleSentrySubsystem::InitWithSettings(settings, beforeSendHandler, beforeBreadcrumbHandler, traceSampler);
}

void FIOSSentrySubsystem::HandleAssert()
{
	if (isScreenshotAttachmentEnabled)
	{
		TryCaptureScreenshot();
	}
}

FString FIOSSentrySubsystem::TryCaptureScreenshot() const
{
	FString ScreenshotPath = GetScreenshotPath();

	dispatch_sync(dispatch_get_main_queue(), ^{
		UIGraphicsBeginImageContextWithOptions([IOSAppDelegate GetDelegate].RootView.bounds.size, NO, 2.0f);
		[[IOSAppDelegate GetDelegate].RootView drawViewHierarchyInRect:[IOSAppDelegate GetDelegate].RootView.bounds afterScreenUpdates:YES];
		UIImage* image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();

		NSData* ImageData = UIImagePNGRepresentation(image);

		TArray<uint8> ImageBytes;
		uint32 SavedSize = ImageData.length;
		ImageBytes.AddUninitialized(SavedSize);
		FPlatformMemory::Memcpy(ImageBytes.GetData(), [ImageData bytes], SavedSize);

		if (!FFileHelper::SaveArrayToFile(ImageBytes, *ScreenshotPath))
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot to: %s"), *ScreenshotPath);
		}
	});

	return ScreenshotPath;
}

FString FIOSSentrySubsystem::GetGameLogPath() const
{
	const FString& logFilePath = SentryFileUtils::GetGameLogPath();
	return IFileManager::Get().FileExists(*logFilePath) ? logFilePath : NormalizeToPublicIOSPath(logFilePath);
}

FString FIOSSentrySubsystem::GetLatestGameLog() const
{
	const FString logFilePath = SentryFileUtils::GetGameLogBackupPath();
	return IFileManager::Get().FileExists(*logFilePath) ? logFilePath : NormalizeToPublicIOSPath(logFilePath);
}

FString FIOSSentrySubsystem::NormalizeToPublicIOSPath(const FString& logFilePath) const
{
	// This is a workaround for iOS log file not being accessible via the path returned by engine's API.
	// See https://github.com/getsentry/sentry-unreal/pull/732

	static FString PublicWritePathBase = FString([NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]);
	static FString PrivateWritePathBase = FString([NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0]);

	if (logFilePath.StartsWith(PrivateWritePathBase))
	{
		return logFilePath.Replace(*PrivateWritePathBase, *PublicWritePathBase);
	}

	return logFilePath;
}
