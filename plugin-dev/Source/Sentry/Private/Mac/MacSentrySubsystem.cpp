// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "Mac/MacSentrySubsystem.h"

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Misc/Paths.h"

#if USE_SENTRY_NATIVE

#include "GenericPlatform/GenericPlatformOutputDevices.h"

void FMacSentrySubsystem::InitWithSettings(const USentrySettings* Settings, const FSentryCallbackHandlers& CallbackHandlers)
{
	FGenericPlatformSentrySubsystem::InitWithSettings(Settings, CallbackHandlers);

	if (Settings->EnableExternalCrashReporter)
	{
		ConfigureCrashReporterAppearance(Settings);
	}

	if (Settings->EnableCrashReporterContextPropagation)
	{
		InitCrashReporter(Settings->GetEffectiveRelease(), Settings->GetEffectiveEnvironment());
	}
}

FString FMacSentrySubsystem::GetHandlerExecutableName() const
{
	return TEXT("sentry-crash");
}

void FMacSentrySubsystem::ConfigureHandlerPath(sentry_options_t* Options)
{
	const FString HandlerPath = GetHandlerPath();

	if (!FPaths::FileExists(HandlerPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Crash handler executable couldn't be found at: %s"), *HandlerPath);
		return;
	}

	sentry_options_set_handler_path(Options, TCHAR_TO_UTF8(*HandlerPath));
}

void FMacSentrySubsystem::ConfigureDatabasePath(sentry_options_t* Options)
{
	sentry_options_set_database_path(Options, TCHAR_TO_UTF8(*GetDatabasePath()));
}

void FMacSentrySubsystem::ConfigureLogFileAttachment(sentry_options_t* Options)
{
	const FString LogFilePath = FGenericPlatformOutputDevices::GetAbsoluteLogFilename();
	sentry_options_add_attachment(Options, TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(LogFilePath)));
}

void FMacSentrySubsystem::ConfigureCrashReporterPath(sentry_options_t* Options)
{
	const FString CrashReporterPath = GetCrashReporterPath();
	if (!FPaths::FileExists(CrashReporterPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("External crash reporter executable couldn't be found at: %s"), *CrashReporterPath);
		return;
	}
	sentry_options_set_external_crash_reporter_path(Options, TCHAR_TO_UTF8(*CrashReporterPath));
}

#else

#include "AppleSentryId.h"

#include "SentryModule.h"

#include "Utils/SentryFileUtils.h"

#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"

void FMacSentrySubsystem::InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers)
{
	FAppleSentrySubsystem::InitWithSettings(settings, callbackHandlers);

	if (IsEnabled())
	{
		if (isScreenshotAttachmentEnabled && !IsRunningCommandlet())
		{
			OnHandleSystemErrorDelegateHandle = FCoreDelegates::OnHandleSystemError.AddLambda([this]()
			{
				TryCaptureScreenshot();
			});
		}
	}
}

void FMacSentrySubsystem::Close()
{
	if (OnHandleSystemErrorDelegateHandle.IsValid())
	{
		FCoreDelegates::OnHandleSystemError.Remove(OnHandleSystemErrorDelegateHandle);
		OnHandleSystemErrorDelegateHandle.Reset();
	}

	FAppleSentrySubsystem::Close();
}

TSharedPtr<ISentryId> FMacSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	TSharedPtr<ISentryId> id = FAppleSentrySubsystem::CaptureEnsure(type, message);

	if (isScreenshotAttachmentEnabled && !IsRunningCommandlet())
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

#endif
