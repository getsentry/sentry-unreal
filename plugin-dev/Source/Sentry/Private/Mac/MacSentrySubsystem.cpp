#include "Mac/MacSentrySubsystem.h"

#include "SentryIdApple.h"

#include "SentryDefines.h"
#include "SentryModule.h"
#include "SentrySettings.h"

#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void FMacSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
	USentryTraceSampler* TraceSampler)
{
	FAppleSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);

	isScreenshotAttachmentEnabled = Settings->AttachScreenshot;

	if (Settings->AttachScreenshot)
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
		TryCaptureScreenshot();
		UploadScreenshotForEvent(id);
	}

	return id;
}

void FMacSentrySubsystem::TryCaptureScreenshot() const
{
	NSWindow* MainWindow = [NSApp mainWindow];
	if (!MainWindow)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("No main window found!"));
		return;
	}

	NSRect WindowRect = [MainWindow frame];
	CGWindowID WindowID = (CGWindowID)[MainWindow windowNumber];
	CGImageRef ScreenshotRef = CGWindowListCreateImage(WindowRect, kCGWindowListOptionIncludingWindow, WindowID, kCGWindowImageDefault);

	if (!ScreenshotRef)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to capture screenshot - invalid ScreenshotRef."));
		return;
	}

	NSBitmapImageRep* BitmapRep = [[NSBitmapImageRep alloc] initWithCGImage:ScreenshotRef];
	NSData* ImageData = [BitmapRep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];

	TArray<uint8> ImageBytes;
	uint32 SavedSize = (uint32)[ImageData length];
	ImageBytes.AddUninitialized(SavedSize);
	FPlatformMemory::Memcpy(ImageBytes.GetData(), [ImageData bytes], SavedSize);

	FString FilePath = GetScreenshotPath();

	if (FFileHelper::SaveArrayToFile(ImageBytes, *FilePath))
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Screenshot saved to: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot to: %s"), *FilePath);
	}

	CGImageRelease(ScreenshotRef);
}

FString FMacSentrySubsystem::GetScreenshotPath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("screenshot.png"));
}
