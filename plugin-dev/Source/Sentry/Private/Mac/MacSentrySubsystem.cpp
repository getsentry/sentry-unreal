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
		TryCaptureScreenshot(id);
		UploadScreenshotForEvent(id);
	}

	return id;
}

void FMacSentrySubsystem::TryCaptureScreenshot(TSharedPtr<ISentryId> eventId) const
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

	CGImageRelease(ScreenshotRef);

	FString ScreenshotPath = GetScreenshotPath(eventId);

	if (!FFileHelper::SaveArrayToFile(ImageBytes, *ScreenshotPath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot to: %s"), *ScreenshotPath);
	}
}
