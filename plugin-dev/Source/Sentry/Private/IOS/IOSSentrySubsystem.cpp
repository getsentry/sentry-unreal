#include "IOS/IOSSentrySubsystem.h"

#include "IOS/IOSAppDelegate.h"

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void FIOSSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
	USentryTraceSampler* TraceSampler)
{
	FAppleSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);
}

void FIOSSentrySubsystem::TryCaptureScreenshot() const
{
	dispatch_sync(dispatch_get_main_queue(), ^{
		UIGraphicsBeginImageContextWithOptions([IOSAppDelegate GetDelegate].RootView.bounds.size, NO, 2.0f);
		[[IOSAppDelegate GetDelegate].RootView drawViewHierarchyInRect:[IOSAppDelegate GetDelegate].RootView.bounds afterScreenUpdates:YES];
		UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();

		NSData *ImageData = UIImagePNGRepresentation(image);

		TArray<uint8> ImageBytes;
		uint32 SavedSize = ImageData.length;
		ImageBytes.AddUninitialized(SavedSize);
		FPlatformMemory::Memcpy(ImageBytes.GetData(), [ImageData bytes], SavedSize);

		FString FilePath = GetScreenshotPath();

		if (FFileHelper::SaveArrayToFile(ImageBytes, *FilePath))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("Screenshot saved to: %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot."));
		}
	});
}

FString FIOSSentrySubsystem::GetScreenshotPath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("screenshot.png"));
}
