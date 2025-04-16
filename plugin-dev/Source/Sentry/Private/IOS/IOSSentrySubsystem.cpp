#include "IOS/IOSSentrySubsystem.h"

#include "IOS/IOSAppDelegate.h"

#include "SentryDefines.h"
#include "SentrySettings.h"

#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Utils/SentryScreenshotUtils.h"

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

static void IOSSentrySignalHandler(int Signal, siginfo_t *Info, void *Context)
{
	if (GIOSSentrySubsystem && GIOSSentrySubsystem->IsEnabled())
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

void FIOSSentrySubsystem::InitWithSettings(const USentrySettings* Settings, USentryBeforeSendHandler* BeforeSendHandler, USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler,
	USentryTraceSampler* TraceSampler)
{
	GIOSSentrySubsystem = this;

	SaveDefaultSignalHandlers();
	InstallSentrySignalHandler();

	FAppleSentrySubsystem::InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);
}

FString FIOSSentrySubsystem::TryCaptureScreenshot() const
{
	FString ScreenshotPath = GetScreenshotPath();

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

		if (!FFileHelper::SaveArrayToFile(ImageBytes, *ScreenshotPath))
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Failed to save screenshot to: %s"), *ScreenshotPath);
		}
	});

	return ScreenshotPath;
}
